/**
 * YüzGizle Web / PWA — İstemci Taraflı Yüz Gizleme & MEB Standartlaştırma Motoru
 * %100 Çevrimdışı ve Cihaz İçi (KVKK Uyumlu)
 */

// Uygulama Durumu (State)
const state = {
  model: null,
  modelLoading: false,
  images: [], // { id, file, name, img, faces: [], processedBlob: null }
  currentIndex: 0,
  
  // Ayarlar
  method: 'blur', // blur, pixelate, blackbar, emoji
  blurStrength: 30,
  pixelSize: 18,
  selectedEmoji: 'smile.png',
  
  preset: 'meb_manset', // meb_manset, eokul, mebbis, insta_square, insta_portrait, insta_story, original
  
  // Filigran & Logo
  watermarkEnabled: false,
  watermarkText: 'T.C. Millî Eğitim Bakanlığı',
  watermarkOpacity: 0.85,
  watermarkPosition: 'bottom_right',
  logoImg: null,

  // Çizim / Etkileşim
  scaleFactor: 1,
  offsetX: 0,
  offsetY: 0,
  isDrawing: false,
  dragStartX: 0,
  dragStartY: 0
};

// =============================================================================
// 1. PWA & Model Başlatma
// =============================================================================
document.addEventListener('DOMContentLoaded', async () => {
  initPWA();
  initTabs();
  initEventListeners();
  await loadFaceModel();
});

function initPWA() {
  if ('serviceWorker' in navigator) {
    navigator.serviceWorker.register('./sw.js')
      .then(reg => console.log('PWA ServiceWorker kayıtlı:', reg.scope))
      .catch(err => console.log('ServiceWorker hatası:', err));
  }

  let deferredPrompt;
  const btnInstall = document.getElementById('btnInstallPwa');
  window.addEventListener('beforeinstallprompt', (e) => {
    e.preventDefault();
    deferredPrompt = e;
    if (btnInstall) btnInstall.style.display = 'inline-flex';
  });

  if (btnInstall) {
    btnInstall.addEventListener('click', async () => {
      if (deferredPrompt) {
        deferredPrompt.prompt();
        const { outcome } = await deferredPrompt.userChoice;
        if (outcome === 'accepted') {
          btnInstall.style.display = 'none';
        }
        deferredPrompt = null;
      }
    });
  }
}

async function loadFaceModel() {
  const loading = document.getElementById('loadingOverlay');
  const loadingText = document.getElementById('loadingText');
  try {
    state.modelLoading = true;
    if (loading) {
      loading.style.display = 'flex';
      loadingText.textContent = 'Yapay zeka yüz algılama modeli yükleniyor...';
    }

    if (window.blazeface) {
      state.model = await blazeface.load();
      console.log('BlazeFace modeli başarıyla yüklendi.');
    } else {
      console.warn('BlazeFace kütüphanesi bulunamadı, manuel seçim modu aktif.');
    }
  } catch (error) {
    console.error('Model yüklenemedi:', error);
  } finally {
    state.modelLoading = false;
    if (loading) loading.style.display = 'none';
  }
}

// =============================================================================
// 2. Sekmeler & Arayüz Kontrolleri
// =============================================================================
function initTabs() {
  const tabButtons = document.querySelectorAll('.tab-btn');
  tabButtons.forEach(btn => {
    btn.addEventListener('click', () => {
      tabButtons.forEach(b => b.classList.remove('active'));
      document.querySelectorAll('.tab-content').forEach(c => c.classList.remove('active'));

      btn.classList.add('active');
      const targetId = btn.getAttribute('data-tab');
      const targetContent = document.getElementById(targetId);
      if (targetContent) targetContent.classList.add('active');
    });
  });
}

function initEventListeners() {
  const fileInput = document.getElementById('fileInput');
  const cameraInput = document.getElementById('cameraInput');
  const dropzone = document.getElementById('uploadDropzone');

  // Dosya Seçimleri
  if (fileInput) fileInput.addEventListener('change', handleFiles);
  if (cameraInput) cameraInput.addEventListener('change', handleFiles);

  // Drag & Drop
  if (dropzone) {
    dropzone.addEventListener('dragover', (e) => {
      e.preventDefault();
      dropzone.classList.add('drag-over');
    });
    dropzone.addEventListener('dragleave', () => dropzone.classList.remove('drag-over'));
    dropzone.addEventListener('drop', (e) => {
      e.preventDefault();
      dropzone.classList.remove('drag-over');
      if (e.dataTransfer.files && e.dataTransfer.files.length > 0) {
        handleFileList(e.dataTransfer.files);
      }
    });
  }

  // Gizleme Yöntemi Kartları
  document.querySelectorAll('.choice-card[data-method]').forEach(card => {
    card.addEventListener('click', () => {
      document.querySelectorAll('.choice-card[data-method]').forEach(c => c.classList.remove('selected'));
      card.classList.add('selected');
      state.method = card.getAttribute('data-method');

      // İlgili ayar panelini aç/kapat
      document.getElementById('blurSettings').style.display = (state.method === 'blur') ? 'flex' : 'none';
      document.getElementById('pixelSettings').style.display = (state.method === 'pixelate') ? 'flex' : 'none';
      document.getElementById('emojiSettings').style.display = (state.method === 'emoji') ? 'flex' : 'none';

      renderCurrentCanvas();
    });
  });

  // Şablon Kartları
  document.querySelectorAll('.choice-card[data-preset]').forEach(card => {
    card.addEventListener('click', () => {
      document.querySelectorAll('.choice-card[data-preset]').forEach(c => c.classList.remove('selected'));
      card.classList.add('selected');
      state.preset = card.getAttribute('data-preset');
      renderCurrentCanvas();
    });
  });

  // Emoji Seçici
  document.querySelectorAll('.emoji-choice').forEach(btn => {
    btn.addEventListener('click', () => {
      document.querySelectorAll('.emoji-choice').forEach(b => b.classList.remove('selected'));
      btn.classList.add('selected');
      state.selectedEmoji = btn.getAttribute('data-emoji');
      renderCurrentCanvas();
    });
  });

  // Slider'lar
  const blurSlider = document.getElementById('blurSlider');
  if (blurSlider) {
    blurSlider.addEventListener('input', (e) => {
      state.blurStrength = parseInt(e.target.value);
      document.getElementById('blurVal').textContent = state.blurStrength;
      renderCurrentCanvas();
    });
  }

  const pixelSlider = document.getElementById('pixelSlider');
  if (pixelSlider) {
    pixelSlider.addEventListener('input', (e) => {
      state.pixelSize = parseInt(e.target.value);
      document.getElementById('pixelVal').textContent = state.pixelSize;
      renderCurrentCanvas();
    });
  }

  // Filigran Kontrolleri
  const chkWatermark = document.getElementById('chkWatermark');
  const txtWatermark = document.getElementById('txtWatermark');
  const logoInput = document.getElementById('logoInput');

  if (chkWatermark) {
    chkWatermark.addEventListener('change', (e) => {
      state.watermarkEnabled = e.target.checked;
      renderCurrentCanvas();
    });
  }

  if (txtWatermark) {
    txtWatermark.addEventListener('input', (e) => {
      state.watermarkText = e.target.value;
      renderCurrentCanvas();
    });
  }

  if (logoInput) {
    logoInput.addEventListener('change', (e) => {
      if (e.target.files && e.target.files[0]) {
        const reader = new FileReader();
        reader.onload = (ev) => {
          const img = new Image();
          img.onload = () => {
            state.logoImg = img;
            renderCurrentCanvas();
          };
          img.src = ev.target.result;
        };
        reader.readAsDataURL(e.target.files[0]);
      }
    });
  }

  // İndirme & Paylaşma Butonları
  const btnDownloadCurrent = document.getElementById('btnDownloadCurrent');
  const btnDownloadAll = document.getElementById('btnDownloadAll');
  const btnShare = document.getElementById('btnShare');

  if (btnDownloadCurrent) btnDownloadCurrent.addEventListener('click', downloadCurrentPhoto);
  if (btnDownloadAll) btnDownloadAll.addEventListener('click', downloadAllPhotosZip);
  if (btnShare) btnShare.addEventListener('click', shareCurrentPhoto);

  // Tuval Dokunmatik & Fare Etkileşimleri
  setupCanvasInteractions();
}

// =============================================================================
// 3. Fotoğraf Yükleme & Yapay Zeka Yüz Tespiti
// =============================================================================
function handleFiles(event) {
  if (event.target.files && event.target.files.length > 0) {
    handleFileList(event.target.files);
  }
}

async function handleFileList(files) {
  const loading = document.getElementById('loadingOverlay');
  const loadingText = document.getElementById('loadingText');
  loading.style.display = 'flex';

  for (let i = 0; i < files.length; i++) {
    const file = files[i];
    if (!file.type.startsWith('image/')) continue;

    loadingText.textContent = `Fotoğraf yükleniyor ve yüzler tespit ediliyor (${i + 1}/${files.length})...`;

    const img = await loadImageFromFile(file);
    const faces = await detectFacesInImage(img);

    state.images.push({
      id: Date.now() + '_' + i,
      file: file,
      name: file.name,
      img: img,
      faces: faces,
      processedBlob: null
    });
  }

  loading.style.display = 'none';

  if (state.images.length > 0) {
    document.getElementById('uploadDropzone').style.display = 'none';
    document.getElementById('canvasWrapper').style.display = 'flex';
    document.getElementById('controlsPanel').style.display = 'block';
    document.getElementById('bottomActionBar').style.display = 'flex';

    if (state.images.length > 1) {
      document.getElementById('photoStrip').style.display = 'flex';
      document.getElementById('btnDownloadAll').style.display = 'inline-flex';
    }

    renderThumbnails();
    state.currentIndex = state.images.length - 1;
    selectImage(state.currentIndex);
  }
}

function loadImageFromFile(file) {
  return new Promise((resolve) => {
    const reader = new FileReader();
    reader.onload = (e) => {
      const img = new Image();
      img.onload = () => resolve(img);
      img.src = e.target.result;
    };
    reader.readAsDataURL(file);
  });
}

async function detectFacesInImage(img) {
  if (!state.model) {
    // Model henüz yoksa yüz listesi boş döner
    return [];
  }

  try {
    const predictions = await state.model.estimateFaces(img, false);
    return predictions.map(pred => {
      const start = pred.topLeft;
      const end = pred.bottomRight;
      const width = end[0] - start[0];
      const height = end[1] - start[1];

      // Pay ekle (baş ve çene tam kapansın)
      const padX = width * 0.1;
      const padY = height * 0.15;

      return {
        x: Math.max(0, start[0] - padX),
        y: Math.max(0, start[1] - padY),
        width: Math.min(img.width, width + (padX * 2)),
        height: Math.min(img.height, height + (padY * 2)),
        exempt: false, // Öğretmen muafiyeti
        manual: false
      };
    });
  } catch (err) {
    console.error('Yüz tespiti hatası:', err);
    return [];
  }
}

// =============================================================================
// 4. Şerit & Görsel Seçimi
// =============================================================================
function renderThumbnails() {
  const strip = document.getElementById('photoStrip');
  strip.innerHTML = '';

  state.images.forEach((item, index) => {
    const thumb = document.createElement('div');
    thumb.className = `thumb-item ${index === state.currentIndex ? 'active' : ''}`;
    thumb.innerHTML = `
      <img src="${item.img.src}" alt="${item.name}">
      <span class="thumb-badge">${item.faces.length}</span>
    `;
    thumb.addEventListener('click', () => selectImage(index));
    strip.appendChild(thumb);
  });
}

function selectImage(index) {
  state.currentIndex = index;
  document.querySelectorAll('.thumb-item').forEach((t, i) => {
    t.classList.toggle('active', i === index);
  });
  renderCurrentCanvas();
}

// =============================================================================
// 5. Tuval Çizimi & Yüz Gizleme & Filigran Motoru
// =============================================================================
function renderCurrentCanvas() {
  const current = state.images[state.currentIndex];
  if (!current) return;

  const canvas = document.getElementById('previewCanvas');
  const ctx = canvas.getContext('2d');

  // Preset Hedef Boyutlarını Belirle
  const targetDims = calculatePresetDimensions(current.img.width, current.img.height, state.preset);
  canvas.width = targetDims.width;
  canvas.height = targetDims.height;

  // 1. Orijinal Resmi Hedef Boyuta Göre Çiz (Cover / Orantılı Kırpma)
  const scale = Math.max(canvas.width / current.img.width, canvas.height / current.img.height);
  const sw = canvas.width / scale;
  const sh = canvas.height / scale;
  const sx = (current.img.width - sw) / 2;
  const sy = (current.img.height - sh) / 2;

  ctx.drawImage(current.img, sx, sy, sw, sh, 0, 0, canvas.width, canvas.height);

  // Koordinat Dönüşüm Oranları
  const scaleX = canvas.width / sw;
  const scaleY = canvas.height / sh;

  // 2. Yüzleri Gizle
  current.faces.forEach((face) => {
    // Fotoğraf içi koordinatları tuval koordinatlarına eşle
    const fx = (face.x - sx) * scaleX;
    const fy = (face.y - sy) * scaleY;
    const fw = face.width * scaleX;
    const fh = face.height * scaleY;

    if (!face.exempt) {
      applyHidingStrategy(ctx, fx, fy, fw, fh);
    }

    // Yüz Kutularını ve Etiketlerini Çiz (Önizleme Rehberi)
    drawFaceGuide(ctx, fx, fy, fw, fh, face);
  });

  // 3. Logo ve Filigran Ekle
  if (state.watermarkEnabled || state.logoImg) {
    applyWatermark(ctx, canvas.width, canvas.height);
  }

  // 4. İstatistik Panelini Güncelle
  updateStatsPill(current.faces);
}

function calculatePresetDimensions(origW, origH, preset) {
  switch (preset) {
    case 'meb_manset':
      return { width: 1200, height: 568 }; // 19:9 MEB Resmi Standardı
    case 'eokul':
      return { width: 133, height: 171 }; // e-Okul Standart Vesikalık
    case 'mebbis':
      return { width: 394, height: 512 }; // MEBBİS Vesikalık
    case 'insta_square':
      return { width: 1080, height: 1080 };
    case 'insta_portrait':
      return { width: 1080, height: 1350 };
    case 'insta_story':
      return { width: 1080, height: 1920 };
    default:
      return { width: origW, height: origH };
  }
}

function applyHidingStrategy(ctx, x, y, w, h) {
  ctx.save();
  ctx.beginPath();
  ctx.rect(x, y, w, h);
  ctx.clip();

  if (state.method === 'blur') {
    // Gaussian Blur
    ctx.filter = `blur(${state.blurStrength}px)`;
    ctx.drawImage(ctx.canvas, 0, 0);
  } else if (state.method === 'pixelate') {
    // Mozaik (Pikselleştirme)
    const size = Math.max(4, state.pixelSize);
    const tempCanvas = document.createElement('canvas');
    tempCanvas.width = Math.max(1, Math.floor(w / size));
    tempCanvas.height = Math.max(1, Math.floor(h / size));
    const tCtx = tempCanvas.getContext('2d');
    tCtx.imageSmoothingEnabled = false;
    tCtx.drawImage(ctx.canvas, x, y, w, h, 0, 0, tempCanvas.width, tempCanvas.height);
    
    ctx.imageSmoothingEnabled = false;
    ctx.drawImage(tempCanvas, 0, 0, tempCanvas.width, tempCanvas.height, x, y, w, h);
  } else if (state.method === 'blackbar') {
    // Siyah Bant
    ctx.fillStyle = '#000000';
    ctx.fillRect(x, y, w, h);
  } else if (state.method === 'emoji') {
    // Emoji ile Kaplama
    const emojiImg = new Image();
    emojiImg.src = `emojis/${state.selectedEmoji}`;
    if (emojiImg.complete) {
      ctx.drawImage(emojiImg, x, y, w, h);
    } else {
      emojiImg.onload = () => renderCurrentCanvas();
    }
  }

  ctx.restore();
}

function drawFaceGuide(ctx, x, y, w, h, face) {
  ctx.save();
  if (face.exempt) {
    // Muaf (Öğretmen) -> Turuncu Kesikli Çerçeve
    ctx.strokeStyle = '#f59e0b';
    ctx.lineWidth = 3;
    ctx.setLineDash([6, 4]);
    ctx.strokeRect(x, y, w, h);

    // Kapsül Etiket
    ctx.fillStyle = 'rgba(245, 158, 11, 0.9)';
    ctx.fillRect(x, Math.max(0, y - 22), 120, 20);
    ctx.fillStyle = '#ffffff';
    ctx.font = 'bold 11px sans-serif';
    ctx.fillText('🛡️ Muaf (Öğretmen)', x + 6, Math.max(14, y - 8));
  } else {
    // Gizlenen Öğrenci Yüzü
    ctx.strokeStyle = '#3b82f6';
    ctx.lineWidth = 2;
    ctx.strokeRect(x, y, w, h);
    if (face.manual) {
      ctx.fillStyle = 'rgba(59, 130, 246, 0.85)';
      ctx.fillRect(x, Math.max(0, y - 18), 28, 16);
      ctx.fillStyle = '#fff';
      ctx.font = 'bold 10px sans-serif';
      ctx.fillText('[M]', x + 5, Math.max(12, y - 6));
    }
  }
  ctx.restore();
}

function applyWatermark(ctx, canvasW, canvasH) {
  ctx.save();
  ctx.globalAlpha = state.watermarkOpacity;

  const pad = 24;
  let posX = canvasW - pad;
  let posY = canvasH - pad;

  // 1. Logo
  if (state.logoImg) {
    const logoW = Math.min(120, canvasW * 0.18);
    const logoH = (state.logoImg.height / state.logoImg.width) * logoW;
    ctx.drawImage(state.logoImg, posX - logoW, posY - logoH, logoW, logoH);
    posY -= (logoH + 8);
  }

  // 2. Metin Filigranı ("T.C. Millî Eğitim Bakanlığı")
  if (state.watermarkEnabled && state.watermarkText.trim() !== '') {
    const fontSize = Math.max(14, Math.floor(canvasW * 0.022));
    ctx.font = `bold ${fontSize}px sans-serif`;
    const textMetrics = ctx.measureText(state.watermarkText);
    const textW = textMetrics.width;
    const textH = fontSize * 1.4;

    const boxX = posX - textW - 16;
    const boxY = posY - textH;

    // Kontrast Kapsülü (Arka plan hapı)
    ctx.fillStyle = 'rgba(15, 23, 42, 0.75)';
    ctx.beginPath();
    ctx.roundRect(boxX, boxY, textW + 16, textH, 6);
    ctx.fill();

    // Metin
    ctx.fillStyle = '#ffffff';
    ctx.fillText(state.watermarkText, boxX + 8, boxY + fontSize);
  }

  ctx.restore();
}

function updateStatsPill(faces) {
  const total = faces.length;
  const exempt = faces.filter(f => f.exempt).length;
  const hidden = total - exempt;

  const pill = document.getElementById('canvasStatsPill');
  if (pill) {
    pill.innerHTML = `
      <span class="stat-item faces">👤 ${hidden} Gizlenecek</span>
      <span class="stat-item exempt">🛡️ ${exempt} Muaf</span>
    `;
  }
}

// =============================================================================
// 6. Tuval Etkileşimi (Dokunarak Muafiyet & Sürükleyerek Alan Ekleme)
// =============================================================================
function setupCanvasInteractions() {
  const canvas = document.getElementById('previewCanvas');
  let startX = 0, startY = 0;
  let isDragging = false;

  const getCanvasCoords = (clientX, clientY) => {
    const rect = canvas.getBoundingClientRect();
    const scaleX = canvas.width / rect.width;
    const scaleY = canvas.height / rect.height;
    return {
      x: (clientX - rect.left) * scaleX,
      y: (clientY - rect.top) * scaleY
    };
  };

  // Tıklama / Dokunma ile Yüz Muafiyeti Aç/Kapat
  const handlePointerDown = (clientX, clientY) => {
    const pos = getCanvasCoords(clientX, clientY);
    startX = pos.x;
    startY = pos.y;
    isDragging = false;
  };

  const handlePointerUp = (clientX, clientY) => {
    const pos = getCanvasCoords(clientX, clientY);
    const dist = Math.hypot(pos.x - startX, pos.y - startY);

    const current = state.images[state.currentIndex];
    if (!current) return;

    if (dist < 10) {
      // Tıklama: Yüz kutusu içinde mi?
      let toggled = false;
      for (let i = current.faces.length - 1; i >= 0; i--) {
        const f = current.faces[i];
        if (pos.x >= f.x && pos.x <= (f.x + f.width) && pos.y >= f.y && pos.y <= (f.y + f.height)) {
          f.exempt = !f.exempt;
          toggled = true;
          break;
        }
      }

      if (toggled) {
        renderCurrentCanvas();
        return;
      }
    } else if (dist >= 20) {
      // Sürükleme: Manuel Gizleme Alanı Ekle
      const rx = Math.min(startX, pos.x);
      const ry = Math.min(startY, pos.y);
      const rw = Math.abs(pos.x - startX);
      const rh = Math.abs(pos.y - startY);

      current.faces.push({
        x: rx,
        y: ry,
        width: rw,
        height: rh,
        exempt: false,
        manual: true
      });

      renderCurrentCanvas();
    }
  };

  canvas.addEventListener('mousedown', (e) => handlePointerDown(e.clientX, e.clientY));
  canvas.addEventListener('mouseup', (e) => handlePointerUp(e.clientX, e.clientY));

  canvas.addEventListener('touchstart', (e) => {
    if (e.touches.length === 1) {
      handlePointerDown(e.touches[0].clientX, e.touches[0].clientY);
    }
  }, { passive: true });

  canvas.addEventListener('touchend', (e) => {
    if (e.changedTouches.length === 1) {
      handlePointerUp(e.changedTouches[0].clientX, e.changedTouches[0].clientY);
    }
  }, { passive: true });
}

// =============================================================================
// 7. İndirme & e-Okul (< 20 KB) Sıkıştırma Motoru
// =============================================================================
async function exportCanvasToBlob(canvas, preset) {
  if (preset === 'eokul') {
    // e-Okul için iteratif < 20 KB JPEG kalitesi arama
    let quality = 0.85;
    let blob = await new Promise(r => canvas.toBlob(r, 'image/jpeg', quality));
    while (blob.size > 20 * 1024 && quality > 0.15) {
      quality -= 0.1;
      blob = await new Promise(r => canvas.toBlob(r, 'image/jpeg', quality));
    }
    return blob;
  }
  return new Promise(r => canvas.toBlob(r, 'image/jpeg', 0.92));
}

async function downloadCurrentPhoto() {
  const current = state.images[state.currentIndex];
  if (!current) return;

  const canvas = document.getElementById('previewCanvas');
  const blob = await exportCanvasToBlob(canvas, state.preset);

  const url = URL.createObjectURL(blob);
  const a = document.createElement('a');
  a.href = url;
  const baseName = current.name.substring(0, current.name.lastIndexOf('.')) || 'foto';
  a.download = `${baseName}_${state.preset}_yuzgizle.jpg`;
  document.body.appendChild(a);
  a.click();
  document.body.removeChild(a);
  URL.revokeObjectURL(url);
}

async function downloadAllPhotosZip() {
  if (!window.JSZip || state.images.length === 0) return;

  const loading = document.getElementById('loadingOverlay');
  const loadingText = document.getElementById('loadingText');
  loading.style.display = 'flex';
  loadingText.textContent = 'Tüm fotoğraflar işlenip ZIP arşivi hazırlanıyor...';

  const zip = new JSZip();
  const folder = zip.folder('YuzGizle_Fotograflar');

  const canvas = document.getElementById('previewCanvas');
  const savedIndex = state.currentIndex;

  for (let i = 0; i < state.images.length; i++) {
    selectImage(i);
    renderCurrentCanvas();
    const blob = await exportCanvasToBlob(canvas, state.preset);
    const baseName = state.images[i].name.substring(0, state.images[i].name.lastIndexOf('.')) || `foto_${i + 1}`;
    folder.file(`${baseName}_${state.preset}.jpg`, blob);
  }

  selectImage(savedIndex);
  loadingText.textContent = 'ZIP dosyası sıkıştırılıyor...';

  const content = await zip.generateAsync({ type: 'blob' });
  const a = document.createElement('a');
  a.href = URL.createObjectURL(content);
  a.download = `YuzGizle_Toplu_${Date.now()}.zip`;
  document.body.appendChild(a);
  a.click();
  document.body.removeChild(a);

  loading.style.display = 'none';
}

async function shareCurrentPhoto() {
  const current = state.images[state.currentIndex];
  if (!current) return;

  const canvas = document.getElementById('previewCanvas');
  const blob = await exportCanvasToBlob(canvas, state.preset);
  const file = new File([blob], `${current.name}_yuzgizle.jpg`, { type: 'image/jpeg' });

  if (navigator.canShare && navigator.canShare({ files: [file] })) {
    try {
      await navigator.share({
        title: 'YüzGizle MEB Uyumlu Fotoğraf',
        text: 'MEB KVKK standartlarına uygun olarak hazırlandı.',
        files: [file]
      });
    } catch (err) {
      console.log('Paylaşım iptal edildi veya desteklenmiyor.');
    }
  } else {
    // Paylaşım desteklenmiyorsa doğrudan indir
    downloadCurrentPhoto();
  }
}
