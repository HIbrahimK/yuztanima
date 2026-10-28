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
  
  // Dokunmatik Araçlar, Seçim & Geri Alma
  activeTool: 'add', // 'add' (Ekle), 'remove' (Çıkar), 'exempt' (Muafiyet)
  selectedFaceIndex: -1, // Seçili alanın indeksi (-1 = seçili yok)
  undoStack: [], // Geri alma geçmişi

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

  // Çizim & Dokunmatik Etkileşim
  isPointerDown: false,
  dragStartCanvasX: 0,
  dragStartCanvasY: 0,
  dragCurrentCanvasX: 0,
  dragCurrentCanvasY: 0
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
      loadingText.textContent = 'Yapay zeka modeli çevrimdışı yükleniyor...';
    }

    if (window.blazeface) {
      // Yerel model dosyasından yükle (Harici sunucu bağımlılığı yok)
      // Kalabalık sınıf fotoğrafları için maxFaces: 100 ve hassas algılama için scoreThreshold: 0.42
      state.model = await blazeface.load({
        modelUrl: './models/model.json',
        maxFaces: 100,
        scoreThreshold: 0.42,
        iouThreshold: 0.3
      });
      console.log('✅ Yerel BlazeFace modeli başarıyla yüklendi (maxFaces: 100, threshold: 0.42).');
    }
  } catch (error) {
    console.warn('Yerel model yüklenirken hata, CDN deneniyor:', error);
    try {
      if (window.blazeface) {
        state.model = await blazeface.load({
          maxFaces: 100,
          scoreThreshold: 0.42,
          iouThreshold: 0.3
        });
      }
    } catch (err2) {
      console.error('Yapay zeka modeli yüklenemedi:', err2);
    }
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

  if (fileInput) fileInput.addEventListener('change', handleFiles);
  if (cameraInput) cameraInput.addEventListener('change', handleFiles);

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

  // Gizleme Yöntemi Seçimi
  document.querySelectorAll('.choice-card[data-method]').forEach(card => {
    card.addEventListener('click', () => {
      document.querySelectorAll('.choice-card[data-method]').forEach(c => c.classList.remove('selected'));
      card.classList.add('selected');
      state.method = card.getAttribute('data-method');

      document.getElementById('blurSettings').style.display = (state.method === 'blur') ? 'flex' : 'none';
      document.getElementById('pixelSettings').style.display = (state.method === 'pixelate') ? 'flex' : 'none';
      document.getElementById('emojiSettings').style.display = (state.method === 'emoji') ? 'flex' : 'none';

      renderCurrentCanvas();
    });
  });

  // Şablon Seçimi
  document.querySelectorAll('.choice-card[data-preset]').forEach(card => {
    card.addEventListener('click', () => {
      document.querySelectorAll('.choice-card[data-preset]').forEach(c => c.classList.remove('selected'));
      card.classList.add('selected');
      state.preset = card.getAttribute('data-preset');
      renderCurrentCanvas();
    });
  });

  // Emoji Seçimi
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

  // Filigran & Logo
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

  // İndirme & Paylaşma
  const btnDownloadCurrent = document.getElementById('btnDownloadCurrent');
  const btnDownloadAll = document.getElementById('btnDownloadAll');
  const btnShare = document.getElementById('btnShare');
  const btnResetPhotos = document.getElementById('btnResetPhotos');

  if (btnDownloadCurrent) btnDownloadCurrent.addEventListener('click', downloadCurrentPhoto);
  if (btnDownloadAll) btnDownloadAll.addEventListener('click', downloadAllPhotosZip);
  if (btnShare) btnShare.addEventListener('click', shareCurrentPhoto);
  if (btnResetPhotos) btnResetPhotos.addEventListener('click', resetToUploadScreen);

  // Dokunmatik Araç Çubuğu (Toolbar)
  const btnToolAdd = document.getElementById('btnToolAdd');
  const btnToolRemove = document.getElementById('btnToolRemove');
  const btnToolExempt = document.getElementById('btnToolExempt');
  const btnUndo = document.getElementById('btnUndo');

  if (btnToolAdd) btnToolAdd.addEventListener('click', () => setActiveTool('add'));
  if (btnToolRemove) {
    btnToolRemove.addEventListener('click', () => {
      // Eğer bir alan seçiliyse doğrudan o alanı çıkar
      if (state.selectedFaceIndex >= 0) {
        deleteSelectedFace();
      } else {
        // Değilse çıkarma moduna geç (üzerine tıklananı sil)
        setActiveTool('remove');
      }
    });
  }
  if (btnToolExempt) btnToolExempt.addEventListener('click', () => setActiveTool('exempt'));
  if (btnUndo) btnUndo.addEventListener('click', undoLastAction);

  // Yüz Tanı / Tarama Butonları
  const btnScanFacesCanvas = document.getElementById('btnScanFacesCanvas');
  const btnScanFaces = document.getElementById('btnScanFaces');
  const btnScanCurrentTab = document.getElementById('btnScanCurrentTab');
  const btnScanAllTab = document.getElementById('btnScanAllTab');

  if (btnScanFacesCanvas) btnScanFacesCanvas.addEventListener('click', scanFacesCurrentPhoto);
  if (btnScanFaces) btnScanFaces.addEventListener('click', scanFacesCurrentPhoto);
  if (btnScanCurrentTab) btnScanCurrentTab.addEventListener('click', scanFacesCurrentPhoto);
  if (btnScanAllTab) btnScanAllTab.addEventListener('click', scanFacesAllPhotos);

  // Seçili Alan Bağlamsal Eylem Çubuğu
  const btnDeleteSelectedFace = document.getElementById('btnDeleteSelectedFace');
  const btnToggleSelectedExempt = document.getElementById('btnToggleSelectedExempt');
  const btnDeselectFace = document.getElementById('btnDeselectFace');

  if (btnDeleteSelectedFace) btnDeleteSelectedFace.addEventListener('click', deleteSelectedFace);
  if (btnToggleSelectedExempt) btnToggleSelectedExempt.addEventListener('click', toggleSelectedExempt);
  if (btnDeselectFace) btnDeselectFace.addEventListener('click', deselectFace);

  // Temizle Butonu
  const btnClearManual = document.getElementById('btnClearManual');
  if (btnClearManual) btnClearManual.addEventListener('click', clearAllManual);

  // Klavye Kısayolları (Masaüstü Web Uyumluluğu)
  window.addEventListener('keydown', handleKeyboardShortcuts);

  // Tuval Dokunmatik & Fare Etkileşimleri
  setupCanvasInteractions();
}

// =============================================================================
// 3. Fotoğraf Yükleme & Yapay Zeka Yüz Tespiti (Ölçeklendirilmiş & Hızlı)
// =============================================================================
function handleFiles(event) {
  if (event.target.files && event.target.files.length > 0) {
    const files = event.target.files;
    handleFileList(files);
    // Aynı veya yeni dosyaların tekrar seçilebilmesi için input değerini sıfırla
    event.target.value = '';
  }
}

async function handleFileList(files) {
  const loading = document.getElementById('loadingOverlay');
  const loadingText = document.getElementById('loadingText');
  if (loading) {
    loading.style.display = 'flex';
    loadingText.textContent = 'Fotoğraflar taranıyor...';
  }

  // Model henüz hazır değilse önce kesinlikle yüklenmesini bekle
  if (!state.model) {
    if (loadingText) loadingText.textContent = 'Yapay zeka modeli hazırlanıyor...';
    await loadFaceModel();
  }

  const prevTotal = state.images.length;
  let addedCount = 0;

  for (let i = 0; i < files.length; i++) {
    const file = files[i];
    if (!file.type.startsWith('image/')) continue;

    if (loadingText) {
      loadingText.textContent = `Yüzler taranıyor (${i + 1}/${files.length}): ${file.name}...`;
    }

    const img = await loadImageFromFile(file);
    if (!img) continue;

    // Yüz tespiti yap (Hassas eşik 0.42, maxFaces 100)
    const faces = await detectFacesInImage(img, 0.42);

    state.images.push({
      id: Date.now() + '_' + i + '_' + Math.random().toString(36).substr(2, 5),
      file: file,
      name: file.name,
      img: img,
      faces: faces,
      processedBlob: null
    });

    addedCount++;
  }

  if (loading) loading.style.display = 'none';

  if (state.images.length > 0) {
    document.getElementById('uploadDropzone').style.display = 'none';
    document.getElementById('canvasWrapper').style.display = 'flex';
    document.getElementById('touchToolbar').style.display = 'flex';
    document.getElementById('canvasModeHint').style.display = 'flex';
    document.getElementById('controlsPanel').style.display = 'block';
    document.getElementById('bottomActionBar').style.display = 'flex';

    const photoStrip = document.getElementById('photoStrip');
    if (photoStrip) photoStrip.style.display = 'flex';

    const btnDownloadAll = document.getElementById('btnDownloadAll');
    if (btnDownloadAll) {
      btnDownloadAll.style.display = (state.images.length > 1) ? 'inline-flex' : 'none';
    }

    renderThumbnails();

    // Sonradan eklenen ilk veya son fotoğrafa odaklan
    const targetIndex = (addedCount > 0) ? (state.images.length - 1) : 0;
    selectImage(targetIndex);
    setActiveTool('add');

    if (addedCount > 0) {
      const detectedFacesCount = state.images[targetIndex].faces.length;
      showToast(`📸 ${addedCount} yeni fotoğraf eklendi (${detectedFacesCount} yüz algılandı).`);
    }
  }
}

function loadImageFromFile(file) {
  return new Promise((resolve) => {
    const reader = new FileReader();
    reader.onload = (e) => {
      const img = new Image();
      img.onload = () => resolve(img);
      img.onerror = (err) => {
        console.error('Görsel yüklenemedi:', err);
        resolve(null);
      };
      img.src = e.target.result;
    };
    reader.onerror = (err) => {
      console.error('Dosya okunamadı:', err);
      resolve(null);
    };
    reader.readAsDataURL(file);
  });
}

async function detectFacesInImage(img, customThreshold = 0.42) {
  // Model yüklenmemişse tekrar yükle
  if (!state.model) {
    console.log('Model henüz hazır değil, yükleniyor...');
    await loadFaceModel();
  }
  if (!state.model) {
    console.warn('Model yüklenemedi.');
    return [];
  }

  try {
    // Model parametrelerini dinamik olarak güncelle
    if (state.model.scoreThreshold !== undefined) {
      state.model.scoreThreshold = customThreshold;
    }
    if (state.model.maxFaces !== undefined) {
      state.model.maxFaces = 100;
    }

    // Telefon fotoğrafları genellikle çok büyüktür (12MP - 48MP)
    // Kalabalık sınıflardaki küçük yüzlerin kaybolmaması için 1440px çözünürlük kullanıyoruz
    const maxDim = 1440;
    let inputSource = img;
    let scaleDet = 1.0;

    if (img.width > maxDim || img.height > maxDim) {
      scaleDet = maxDim / Math.max(img.width, img.height);
      const tempCanvas = document.createElement('canvas');
      tempCanvas.width = Math.round(img.width * scaleDet);
      tempCanvas.height = Math.round(img.height * scaleDet);
      const tempCtx = tempCanvas.getContext('2d');
      tempCtx.drawImage(img, 0, 0, tempCanvas.width, tempCanvas.height);
      inputSource = tempCanvas;
    }

    let predictions = await state.model.estimateFaces(inputSource, false);

    // Eğer hiç yüz bulunamadıysa ve hassasiyet artırılabilirse daha düşük eşikle ikinci geçiş yap
    if (predictions.length === 0 && customThreshold > 0.32) {
      state.model.scoreThreshold = 0.32;
      predictions = await state.model.estimateFaces(inputSource, false);
      state.model.scoreThreshold = customThreshold;
    }

    console.log(`Fotoğrafta ${predictions.length} yüz bulundu.`);

    return predictions.map(pred => {
      // Koordinatları orijinal görüntü boyutuna geri ölçeklendir
      const startX = pred.topLeft[0] / scaleDet;
      const startY = pred.topLeft[1] / scaleDet;
      const endX = pred.bottomRight[0] / scaleDet;
      const endY = pred.bottomRight[1] / scaleDet;

      const width = endX - startX;
      const height = endY - startY;

      // Çene ve saç için güvenlik payı
      const padX = width * 0.12;
      const padY = height * 0.18;

      return {
        x: Math.max(0, startX - padX),
        y: Math.max(0, startY - padY),
        width: Math.min(img.width, width + (padX * 2)),
        height: Math.min(img.height, height + (padY * 2)),
        exempt: false, // Varsayılan: Gizlenecek
        manual: false
      };
    });
  } catch (err) {
    console.error('Yüz tespiti sırasında hata:', err);
    return [];
  }
}

// =============================================================================
// 4. Şerit & Görsel Seçimi
// =============================================================================
function renderThumbnails() {
  const strip = document.getElementById('photoStrip');
  if (!strip) return;
  strip.innerHTML = '';

  state.images.forEach((item, index) => {
    const thumb = document.createElement('div');
    thumb.className = `thumb-item ${index === state.currentIndex ? 'active' : ''}`;
    thumb.innerHTML = `
      <img src="${item.img.src}" alt="${item.name}">
      <span class="thumb-badge">${item.faces.length}</span>
      <button class="thumb-remove" title="Fotoğrafı Kaldır" onclick="event.stopPropagation(); removeImage(${index});">✕</button>
    `;
    thumb.addEventListener('click', () => selectImage(index));
    strip.appendChild(thumb);
  });

  // Şeridin sonuna "+ Ekle" kutucuğu
  const addTile = document.createElement('label');
  addTile.className = 'thumb-add-tile';
  addTile.title = 'Yeni Fotoğraf Ekle';
  addTile.innerHTML = `
    <span style="font-size: 1.2rem;">➕</span>
    <span>Ekle</span>
    <input type="file" accept="image/*" multiple style="display: none;" onchange="handleFiles(event)">
  `;
  strip.appendChild(addTile);
}

function selectImage(index) {
  state.currentIndex = index;
  state.selectedFaceIndex = -1;
  updateSelectedFaceBar();
  document.querySelectorAll('.thumb-item').forEach((t, i) => {
    t.classList.toggle('active', i === index);
  });
  renderCurrentCanvas();
}

// =============================================================================
// 5. Tuval Çizimi & Koordinat Dönüşüm Matrisi (Kritik Düzeltme!)
// =============================================================================

/**
 * Tuval boyutu ile orijinal görüntü arasındaki ölçek ve kırpma (crop) değerlerini hesaplar.
 */
function getCanvasTransform(current, canvas) {
  const targetDims = calculatePresetDimensions(current.img.width, current.img.height, state.preset);
  canvas.width = targetDims.width;
  canvas.height = targetDims.height;

  // Cover modu: Görüntü hedef en/boy oranına orantılı olarak sığdırılır
  const scale = Math.max(canvas.width / current.img.width, canvas.height / current.img.height);
  const sw = canvas.width / scale;
  const sh = canvas.height / scale;
  const sx = (current.img.width - sw) / 2;
  const sy = (current.img.height - sh) / 2;

  return { scale, sw, sh, sx, sy, targetDims };
}

/**
 * Orijinal görüntü piksel koordinatını Tuval piksel koordinatına dönüştürür
 */
function imageToCanvasRect(face, transform, canvas) {
  const scaleX = canvas.width / transform.sw;
  const scaleY = canvas.height / transform.sh;
  return {
    x: (face.x - transform.sx) * scaleX,
    y: (face.y - transform.sy) * scaleY,
    width: face.width * scaleX,
    height: face.height * scaleY
  };
}

/**
 * Tuval piksel koordinatını Orijinal görüntü piksel koordinatına dönüştürür (Manuel Seçim İçin)
 */
function canvasToImageRect(canvasRect, transform, canvas) {
  const invScaleX = transform.sw / canvas.width;
  const invScaleY = transform.sh / canvas.height;
  return {
    x: Math.max(0, transform.sx + canvasRect.x * invScaleX),
    y: Math.max(0, transform.sy + canvasRect.y * invScaleY),
    width: Math.max(10, canvasRect.width * invScaleX),
    height: Math.max(10, canvasRect.height * invScaleY)
  };
}

function renderCurrentCanvas() {
  const current = state.images[state.currentIndex];
  if (!current) return;

  const canvas = document.getElementById('previewCanvas');
  const ctx = canvas.getContext('2d');
  const transform = getCanvasTransform(current, canvas);

  // 1. Resmi Çiz
  ctx.drawImage(current.img, transform.sx, transform.sy, transform.sw, transform.sh, 0, 0, canvas.width, canvas.height);

  // 2. Yüzleri Gizle ve Rehber Çerçeveleri Çiz
  current.faces.forEach((face, idx) => {
    const cRect = imageToCanvasRect(face, transform, canvas);

    if (!face.exempt) {
      applyHidingStrategy(ctx, cRect.x, cRect.y, cRect.width, cRect.height);
    }

    const isSelected = (idx === state.selectedFaceIndex);
    drawFaceGuide(ctx, cRect.x, cRect.y, cRect.width, cRect.height, face, isSelected, state.activeTool, idx);
  });

  // 3. Kullanıcı Parmağıyla/Fareyle Sürükleme Yapıyorsa Canlı Seçim Kutusunu Çiz
  if (state.isPointerDown) {
    const rx = Math.min(state.dragStartCanvasX, state.dragCurrentCanvasX);
    const ry = Math.min(state.dragStartCanvasY, state.dragCurrentCanvasY);
    const rw = Math.abs(state.dragCurrentCanvasX - state.dragStartCanvasX);
    const rh = Math.abs(state.dragCurrentCanvasY - state.dragStartCanvasY);

    if (rw > 5 || rh > 5) {
      ctx.save();
      ctx.strokeStyle = '#facc15';
      ctx.lineWidth = 3;
      ctx.setLineDash([6, 6]);
      ctx.strokeRect(rx, ry, rw, rh);
      ctx.fillStyle = 'rgba(250, 204, 21, 0.2)';
      ctx.fillRect(rx, ry, rw, rh);

      // Boyut Bilgi Hapı
      ctx.fillStyle = 'rgba(15, 23, 42, 0.9)';
      ctx.fillRect(rx, Math.max(0, ry - 22), 95, 18);
      ctx.fillStyle = '#facc15';
      ctx.font = 'bold 11px sans-serif';
      ctx.fillText('Yeni Alan...', rx + 6, Math.max(14, ry - 8));
      ctx.restore();
    }
  }

  // 4. Logo ve Filigran
  if (state.watermarkEnabled || state.logoImg) {
    applyWatermark(ctx, canvas.width, canvas.height);
  }

  // 5. Durum Hapını Güncelle
  updateStatsPill(current.faces);
}

function calculatePresetDimensions(origW, origH, preset) {
  switch (preset) {
    case 'meb_manset':
      return { width: 1200, height: 568 }; // 19:9 MEB Standardı
    case 'eokul':
      return { width: 133, height: 171 }; // e-Okul Standart
    case 'mebbis':
      return { width: 394, height: 512 }; // MEBBİS
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
    ctx.filter = `blur(${state.blurStrength}px)`;
    ctx.drawImage(ctx.canvas, 0, 0);
  } else if (state.method === 'pixelate') {
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
    ctx.fillStyle = '#000000';
    ctx.fillRect(x, y, w, h);
  } else if (state.method === 'emoji') {
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

function drawFaceGuide(ctx, x, y, w, h, face, isSelected = false, activeTool = 'add', faceIdx = 0) {
  ctx.save();

  if (isSelected) {
    // 1. Seçili Alan: Parlak Cam Göbeği / Sky Blue Çerçeve, Gölge & Köşe Tutamaçları
    ctx.shadowColor = 'rgba(56, 189, 248, 0.85)';
    ctx.shadowBlur = 10;
    ctx.strokeStyle = '#38bdf8';
    ctx.lineWidth = 3;
    ctx.setLineDash([6, 4]);
    ctx.strokeRect(x, y, w, h);
    ctx.shadowBlur = 0;

    // Yarı saydam seçim dolgusu
    ctx.fillStyle = 'rgba(56, 189, 248, 0.12)';
    ctx.fillRect(x, y, w, h);

    // 4 Köşe Tutamacı
    const handleSize = 6;
    ctx.fillStyle = '#ffffff';
    ctx.strokeStyle = '#0284c7';
    ctx.lineWidth = 1.5;
    ctx.setLineDash([]);
    const corners = [
      [x, y],
      [x + w, y],
      [x, y + h],
      [x + w, y + h]
    ];
    corners.forEach(([cx, cy]) => {
      ctx.fillRect(cx - handleSize / 2, cy - handleSize / 2, handleSize, handleSize);
      ctx.strokeRect(cx - handleSize / 2, cy - handleSize / 2, handleSize, handleSize);
    });

    // Başlık Rozeti
    ctx.fillStyle = 'rgba(14, 165, 233, 0.95)';
    ctx.beginPath();
    ctx.roundRect(x, Math.max(0, y - 22), 96, 20, 4);
    ctx.fill();
    ctx.fillStyle = '#ffffff';
    ctx.font = 'bold 11px sans-serif';
    ctx.fillText(`🎯 Seçili #${faceIdx + 1}`, x + 6, Math.max(14, y - 8));
  } else if (face.exempt) {
    // 2. Muafiyetli Yüz: Amber / Turuncu
    ctx.strokeStyle = '#f59e0b';
    ctx.lineWidth = 2.5;
    ctx.setLineDash([6, 4]);
    ctx.strokeRect(x, y, w, h);

    ctx.fillStyle = 'rgba(245, 158, 11, 0.95)';
    ctx.beginPath();
    ctx.roundRect(x, Math.max(0, y - 22), 125, 20, 4);
    ctx.fill();
    ctx.fillStyle = '#ffffff';
    ctx.font = 'bold 11px sans-serif';
    ctx.fillText('🛡️ Muaf (Öğretmen)', x + 6, Math.max(14, y - 8));
  } else {
    // 3. Normal Gizlenen Yüz
    ctx.strokeStyle = '#3b82f6';
    ctx.lineWidth = 2;
    ctx.strokeRect(x, y, w, h);
    if (face.manual) {
      ctx.fillStyle = 'rgba(59, 130, 246, 0.9)';
      ctx.beginPath();
      ctx.roundRect(x, Math.max(0, y - 18), 34, 16, 3);
      ctx.fill();
      ctx.fillStyle = '#fff';
      ctx.font = 'bold 10px sans-serif';
      ctx.fillText('[M]', x + 8, Math.max(12, y - 6));
    }
  }

  // 4. Eğer "Çıkar" Modu aktifse her kutucuğun sağ üstünde kırmızı silme rozeti göster
  if (activeTool === 'remove') {
    ctx.fillStyle = 'rgba(239, 68, 68, 0.95)';
    ctx.beginPath();
    ctx.roundRect(x + w - 46, Math.max(0, y - 22), 46, 20, 4);
    ctx.fill();
    ctx.fillStyle = '#ffffff';
    ctx.font = 'bold 11px sans-serif';
    ctx.fillText('🗑️ Sil', x + w - 40, Math.max(14, y - 8));
  }

  ctx.restore();
}

function applyWatermark(ctx, canvasW, canvasH) {
  ctx.save();
  ctx.globalAlpha = state.watermarkOpacity;

  const pad = 24;
  let posX = canvasW - pad;
  let posY = canvasH - pad;

  if (state.logoImg) {
    const logoW = Math.min(120, canvasW * 0.18);
    const logoH = (state.logoImg.height / state.logoImg.width) * logoW;
    ctx.drawImage(state.logoImg, posX - logoW, posY - logoH, logoW, logoH);
    posY -= (logoH + 8);
  }

  if (state.watermarkEnabled && state.watermarkText.trim() !== '') {
    const fontSize = Math.max(14, Math.floor(canvasW * 0.022));
    ctx.font = `bold ${fontSize}px sans-serif`;
    const textMetrics = ctx.measureText(state.watermarkText);
    const textW = textMetrics.width;
    const textH = fontSize * 1.4;

    const boxX = posX - textW - 16;
    const boxY = posY - textH;

    ctx.fillStyle = 'rgba(15, 23, 42, 0.75)';
    ctx.beginPath();
    ctx.roundRect(boxX, boxY, textW + 16, textH, 6);
    ctx.fill();

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
      <span class="clear-manual-btn" id="btnClearManual" title="Tüm manuel alanları temizle">↺ Temizle</span>
    `;
    const btnClear = document.getElementById('btnClearManual');
    if (btnClear) {
      btnClear.onclick = (e) => {
        e.stopPropagation();
        const current = state.images[state.currentIndex];
        if (current) {
          current.faces = current.faces.filter(f => !f.manual);
          renderCurrentCanvas();
        }
      };
    }
  }
}

// =============================================================================
// 6. Tuval Etkileşimi (Pixel-Perfect Koordinat Eşleme)
// =============================================================================
function setupCanvasInteractions() {
  const canvas = document.getElementById('previewCanvas');

  // Ekran dokunmatik/fare koordinatını doğrudan tuval iç piksel koordinatına çevirir
  const getCanvasPixelCoords = (clientX, clientY) => {
    const rect = canvas.getBoundingClientRect();
    const scaleX = canvas.width / rect.width;
    const scaleY = canvas.height / rect.height;
    return {
      x: (clientX - rect.left) * scaleX,
      y: (clientY - rect.top) * scaleY
    };
  };

  const onPointerDown = (clientX, clientY) => {
    const pos = getCanvasPixelCoords(clientX, clientY);
    state.isPointerDown = true;
    state.dragStartCanvasX = pos.x;
    state.dragStartCanvasY = pos.y;
    state.dragCurrentCanvasX = pos.x;
    state.dragCurrentCanvasY = pos.y;
  };

  const onPointerMove = (clientX, clientY) => {
    if (!state.isPointerDown) return;
    const pos = getCanvasPixelCoords(clientX, clientY);
    state.dragCurrentCanvasX = pos.x;
    state.dragCurrentCanvasY = pos.y;
    renderCurrentCanvas();
  };

  const onPointerUp = (clientX, clientY, isShiftHeld = false) => {
    if (!state.isPointerDown) return;
    state.isPointerDown = false;

    const current = state.images[state.currentIndex];
    if (!current) return;

    const pos = getCanvasPixelCoords(clientX, clientY);
    const dist = Math.hypot(pos.x - state.dragStartCanvasX, pos.y - state.dragStartCanvasY);
    const transform = getCanvasTransform(current, canvas);

    if (dist < 15) {
      // 1. TIKLAMA / DOKUNMA: Bir yüzün veya alanın üzerine mi tıklandı?
      let clickedIdx = -1;
      for (let i = current.faces.length - 1; i >= 0; i--) {
        const face = current.faces[i];
        const cRect = imageToCanvasRect(face, transform, canvas);

        if (
          pos.x >= cRect.x &&
          pos.x <= (cRect.x + cRect.width) &&
          pos.y >= cRect.y &&
          pos.y <= (cRect.y + cRect.height)
        ) {
          clickedIdx = i;
          break;
        }
      }

      if (clickedIdx >= 0) {
        // Shift tuşuna basılıysa VEYA aktif mod 'remove' (Çıkar) ise doğrudan sil
        if (isShiftHeld || state.activeTool === 'remove') {
          deleteFace(clickedIdx);
        } else if (state.activeTool === 'exempt') {
          // Muafiyet modundaysa muafiyeti aç/kapat
          toggleFaceExempt(clickedIdx);
        } else {
          // Normal / Ekle Modu:
          if (state.selectedFaceIndex === clickedIdx) {
            // Zaten seçiliyse muafiyeti aç/kapat
            toggleFaceExempt(clickedIdx);
          } else {
            // Alanı seç ve bağlamsal çubuğu aç
            selectFace(clickedIdx);
          }
        }
      } else {
        // Boş alana tıklandıysa seçimi kaldır
        deselectFace();
      }
      return;
    } else if (dist >= 18) {
      // 2. SÜRÜKLEME: Manuel Gizleme Alanı Ekle
      if (state.activeTool === 'add' || state.activeTool === 'exempt') {
        const minX = Math.min(state.dragStartCanvasX, pos.x);
        const minY = Math.min(state.dragStartCanvasY, pos.y);
        const w = Math.abs(pos.x - state.dragStartCanvasX);
        const h = Math.abs(pos.y - state.dragStartCanvasY);

        if (w >= 15 && h >= 15) {
          saveUndoSnapshot('Manuel Alan Ekle');
          const imgRect = canvasToImageRect({ x: minX, y: minY, width: w, height: h }, transform, canvas);

          current.faces.push({
            x: imgRect.x,
            y: imgRect.y,
            width: imgRect.width,
            height: imgRect.height,
            exempt: false,
            manual: true
          });

          selectFace(current.faces.length - 1);
          showToast('➕ Yeni gizleme alanı eklendi.');
          renderCurrentCanvas();
        }
      }
    }

    renderCurrentCanvas();
  };

  // Fare Etkinlikleri
  canvas.addEventListener('mousedown', (e) => onPointerDown(e.clientX, e.clientY));
  window.addEventListener('mousemove', (e) => onPointerMove(e.clientX, e.clientY));
  window.addEventListener('mouseup', (e) => onPointerUp(e.clientX, e.clientY, e.shiftKey));

  // Dokunmatik Etkinlikleri (Mobil)
  canvas.addEventListener('touchstart', (e) => {
    if (e.touches.length === 1) {
      onPointerDown(e.touches[0].clientX, e.touches[0].clientY);
    }
  }, { passive: true });

  canvas.addEventListener('touchmove', (e) => {
    if (e.touches.length === 1 && state.isPointerDown) {
      onPointerMove(e.touches[0].clientX, e.touches[0].clientY);
    }
  }, { passive: true });

  canvas.addEventListener('touchend', (e) => {
    if (e.changedTouches.length === 1) {
      onPointerUp(e.changedTouches[0].clientX, e.changedTouches[0].clientY, false);
    }
  }, { passive: true });
}

// =============================================================================
// Yüz Tanıma & Yeniden Tarama (Yüz Tanı)
// =============================================================================

async function scanFacesCurrentPhoto() {
  const current = state.images[state.currentIndex];
  if (!current) {
    showToast('⚠️ Lütfen önce bir fotoğraf yükleyin veya seçin.');
    return;
  }

  const loading = document.getElementById('loadingOverlay');
  const loadingText = document.getElementById('loadingText');
  if (loading) {
    loading.style.display = 'flex';
    loadingText.textContent = 'Yüzler yapay zeka ile taranıyor...';
  }

  // Model hazır değilse yükle
  if (!state.model) {
    await loadFaceModel();
  }

  saveUndoSnapshot('Yüzleri Yeniden Tara');

  // Kullanıcının elle eklediği manuel gizleme alanlarını koru
  const existingManual = current.faces.filter(f => f.manual);

  // Yüzleri tara (Hassasiyet 0.40)
  const newDetectedFaces = await detectFacesInImage(current.img, 0.40);

  current.faces = [...newDetectedFaces, ...existingManual];
  state.selectedFaceIndex = -1;
  updateSelectedFaceBar();
  renderThumbnails();
  renderCurrentCanvas();

  if (loading) loading.style.display = 'none';

  if (navigator.vibrate) navigator.vibrate([40, 30, 40]);

  if (newDetectedFaces.length > 0) {
    showToast(`✅ Yüz taraması tamamlandı: ${newDetectedFaces.length} yüz bulundu.`, 'Geri Al', undoLastAction);
  } else {
    showToast('ℹ️ Fotoğrafta otomatik yüz bulunamadı. "➕ Ekle" butonuyla elle seçebilirsiniz.');
  }
}

async function scanFacesAllPhotos() {
  if (state.images.length === 0) {
    showToast('⚠️ Taranacak fotoğraf yok.');
    return;
  }

  const loading = document.getElementById('loadingOverlay');
  const loadingText = document.getElementById('loadingText');
  if (loading) loading.style.display = 'flex';

  if (!state.model) {
    if (loadingText) loadingText.textContent = 'Yapay zeka modeli hazırlanıyor...';
    await loadFaceModel();
  }

  saveUndoSnapshot('Tüm Fotoğraflar Yeniden Tarandı');

  let totalFound = 0;
  for (let i = 0; i < state.images.length; i++) {
    const item = state.images[i];
    if (loadingText) loadingText.textContent = `Yüzler taranıyor (${i + 1}/${state.images.length}): ${item.name}...`;

    const existingManual = item.faces.filter(f => f.manual);
    const newFaces = await detectFacesInImage(item.img, 0.40);
    item.faces = [...newFaces, ...existingManual];
    totalFound += newFaces.length;
  }

  if (loading) loading.style.display = 'none';
  renderThumbnails();
  renderCurrentCanvas();
  showToast(`✅ Tüm fotoğraflar tarandı: Toplam ${totalFound} yüz tespit edildi.`, 'Geri Al', undoLastAction);
}

// =============================================================================
// Dokunmatik Araçlar, Seçim, Çıkarma & Geri Alma Motoru
// =============================================================================

function setActiveTool(tool) {
  state.activeTool = tool;

  const btnAdd = document.getElementById('btnToolAdd');
  const btnRemove = document.getElementById('btnToolRemove');
  const btnExempt = document.getElementById('btnToolExempt');

  if (btnAdd) btnAdd.classList.toggle('active', tool === 'add');
  if (btnRemove) btnRemove.classList.toggle('active', tool === 'remove');
  if (btnExempt) btnExempt.classList.toggle('active', tool === 'exempt');

  const banner = document.getElementById('canvasModeHint');
  const hintIcon = document.getElementById('hintIcon');
  const hintText = document.getElementById('hintText');

  if (banner && hintText && hintIcon) {
    banner.className = `canvas-mode-banner mode-${tool}`;
    if (tool === 'add') {
      hintIcon.textContent = '💡';
      hintText.innerHTML = '<b>Ekleme Modu:</b> Parmağınızla tuval üzerinde sürükleyerek yeni gizleme alanı çizin.';
    } else if (tool === 'remove') {
      hintIcon.textContent = '🗑️';
      hintText.innerHTML = '<b>Çıkarma Modu:</b> Silmek istediğiniz yüzün veya alanın üzerine dokunun.';
    } else if (tool === 'exempt') {
      hintIcon.textContent = '🛡️';
      hintText.innerHTML = '<b>Muafiyet Modu:</b> Öğretmen vb. gizlenmeyecek yüzün üzerine dokunun.';
    }
  }

  renderCurrentCanvas();
}

function saveUndoSnapshot(label = 'İşlem') {
  const current = state.images[state.currentIndex];
  if (!current) return;

  state.undoStack.push({
    imageIndex: state.currentIndex,
    faces: JSON.parse(JSON.stringify(current.faces)),
    label: label
  });

  if (state.undoStack.length > 25) {
    state.undoStack.shift();
  }

  updateUndoButtonState();
}

function undoLastAction() {
  if (state.undoStack.length === 0) return;

  const snapshot = state.undoStack.pop();
  if (snapshot && state.images[snapshot.imageIndex]) {
    state.currentIndex = snapshot.imageIndex;
    state.images[snapshot.imageIndex].faces = snapshot.faces;
    state.selectedFaceIndex = -1;
    updateSelectedFaceBar();
    updateUndoButtonState();
    renderCurrentCanvas();
    showToast(`↩️ "${snapshot.label}" geri alındı.`);
  }
}

function updateUndoButtonState() {
  const btnUndo = document.getElementById('btnUndo');
  if (btnUndo) {
    btnUndo.disabled = (state.undoStack.length === 0);
  }
}

function deleteFace(index) {
  const current = state.images[state.currentIndex];
  if (!current || index < 0 || index >= current.faces.length) return;

  const isManual = current.faces[index].manual;
  saveUndoSnapshot(isManual ? 'Manuel Alan Silindi' : 'Yüz Silindi');

  current.faces.splice(index, 1);
  state.selectedFaceIndex = -1;
  updateSelectedFaceBar();

  if (navigator.vibrate) navigator.vibrate([30, 30]);

  showToast('🗑️ Alan başarıyla çıkarıldı.', 'Geri Al', undoLastAction);
  renderCurrentCanvas();
}

function deleteSelectedFace() {
  if (state.selectedFaceIndex >= 0) {
    deleteFace(state.selectedFaceIndex);
  } else {
    showToast('⚠️ Lütfen önce çıkarılacak bir alana dokunun.');
  }
}

function toggleFaceExempt(index) {
  const current = state.images[state.currentIndex];
  if (!current || index < 0 || index >= current.faces.length) return;

  saveUndoSnapshot('Muafiyet Değiştirildi');
  current.faces[index].exempt = !current.faces[index].exempt;

  if (navigator.vibrate) navigator.vibrate(20);

  updateSelectedFaceBar();
  renderCurrentCanvas();

  const status = current.faces[index].exempt ? '🛡️ Öğretmen muafiyeti verildi (Gizlenmeyecek)' : '👤 Alan gizlenecek şekilde ayarlandı';
  showToast(status);
}

function toggleSelectedExempt() {
  if (state.selectedFaceIndex >= 0) {
    toggleFaceExempt(state.selectedFaceIndex);
  }
}

function selectFace(index) {
  const current = state.images[state.currentIndex];
  if (!current || index < 0 || index >= current.faces.length) {
    deselectFace();
    return;
  }

  state.selectedFaceIndex = index;
  const face = current.faces[index];

  const bar = document.getElementById('selectedFaceBar');
  const title = document.getElementById('selectedFaceTitle');
  const badge = document.getElementById('selectedFaceBadge');
  const exemptLabel = document.getElementById('exemptActionLabel');

  if (bar && title && badge && exemptLabel) {
    title.textContent = `Alan #${index + 1}`;
    badge.textContent = face.manual ? 'Manuel Alan' : 'Yapay Zeka';
    exemptLabel.textContent = face.exempt ? 'Gizle' : 'Muaf Yap';
    bar.style.display = 'flex';
  }

  if (navigator.vibrate) navigator.vibrate(15);
  renderCurrentCanvas();
}

function deselectFace() {
  state.selectedFaceIndex = -1;
  updateSelectedFaceBar();
  renderCurrentCanvas();
}

function updateSelectedFaceBar() {
  const bar = document.getElementById('selectedFaceBar');
  if (!bar) return;

  if (state.selectedFaceIndex < 0) {
    bar.style.display = 'none';
  } else {
    const current = state.images[state.currentIndex];
    if (current && current.faces[state.selectedFaceIndex]) {
      const face = current.faces[state.selectedFaceIndex];
      const title = document.getElementById('selectedFaceTitle');
      const badge = document.getElementById('selectedFaceBadge');
      const exemptLabel = document.getElementById('exemptActionLabel');

      if (title) title.textContent = `Alan #${state.selectedFaceIndex + 1}`;
      if (badge) badge.textContent = face.manual ? 'Manuel Alan' : 'Yapay Zeka';
      if (exemptLabel) exemptLabel.textContent = face.exempt ? 'Gizle' : 'Muaf Yap';
      bar.style.display = 'flex';
    } else {
      bar.style.display = 'none';
    }
  }
}

function clearAllManual() {
  const current = state.images[state.currentIndex];
  if (!current) return;

  const manualCount = current.faces.filter(f => f.manual).length;
  if (manualCount === 0) {
    showToast('Temizlenecek manuel alan yok.');
    return;
  }

  saveUndoSnapshot('Tüm Manuel Alanlar Silindi');
  current.faces = current.faces.filter(f => !f.manual);
  state.selectedFaceIndex = -1;
  updateSelectedFaceBar();
  renderCurrentCanvas();
  showToast(`↺ ${manualCount} manuel alan temizlendi.`, 'Geri Al', undoLastAction);
}

function removeImage(index) {
  if (index < 0 || index >= state.images.length) return;

  state.images.splice(index, 1);
  if (state.images.length === 0) {
    resetToUploadScreen();
    return;
  }

  if (state.currentIndex >= state.images.length) {
    state.currentIndex = state.images.length - 1;
  }

  state.selectedFaceIndex = -1;
  updateSelectedFaceBar();
  renderThumbnails();
  selectImage(state.currentIndex);

  const btnDownloadAll = document.getElementById('btnDownloadAll');
  if (btnDownloadAll) {
    btnDownloadAll.style.display = (state.images.length > 1) ? 'inline-flex' : 'none';
  }

  showToast('Fotoğraf silindi.');
}

function resetToUploadScreen() {
  state.images = [];
  state.currentIndex = 0;
  state.selectedFaceIndex = -1;
  state.undoStack = [];

  document.getElementById('uploadDropzone').style.display = 'flex';
  document.getElementById('canvasWrapper').style.display = 'none';
  document.getElementById('touchToolbar').style.display = 'none';
  document.getElementById('selectedFaceBar').style.display = 'none';
  document.getElementById('canvasModeHint').style.display = 'none';
  document.getElementById('controlsPanel').style.display = 'none';
  document.getElementById('bottomActionBar').style.display = 'none';
  document.getElementById('photoStrip').style.display = 'none';

  // Dosya girişlerini sıfırla
  const fileInput = document.getElementById('fileInput');
  const cameraInput = document.getElementById('cameraInput');
  if (fileInput) fileInput.value = '';
  if (cameraInput) cameraInput.value = '';

  showToast('Yeni fotoğraflar için karşılama ekranına dönüldü.');
}

function handleKeyboardShortcuts(e) {
  if (e.target.tagName === 'INPUT' || e.target.tagName === 'TEXTAREA') return;

  if (e.key === 'Delete' || e.key === 'Backspace') {
    if (state.selectedFaceIndex >= 0) {
      e.preventDefault();
      deleteSelectedFace();
    }
  } else if ((e.ctrlKey || e.metaKey) && (e.key === 'z' || e.key === 'Z')) {
    e.preventDefault();
    undoLastAction();
  } else if (e.key === 'Escape') {
    deselectFace();
  } else if (e.key === '1') {
    setActiveTool('add');
  } else if (e.key === '2') {
    setActiveTool('remove');
  } else if (e.key === '3') {
    setActiveTool('exempt');
  } else if (e.key === 'r' || e.key === 'R') {
    scanFacesCurrentPhoto();
  }
}

function showToast(message, actionLabel = null, actionCallback = null) {
  const container = document.getElementById('toastContainer');
  if (!container) return;

  container.innerHTML = '';

  const toast = document.createElement('div');
  toast.className = 'toast-item';

  const msgSpan = document.createElement('span');
  msgSpan.className = 'toast-msg';
  msgSpan.innerHTML = message;
  toast.appendChild(msgSpan);

  if (actionLabel && actionCallback) {
    const btn = document.createElement('button');
    btn.className = 'toast-btn';
    btn.textContent = actionLabel;
    btn.onclick = () => {
      actionCallback();
      toast.remove();
    };
    toast.appendChild(btn);
  }

  container.appendChild(toast);

  setTimeout(() => {
    toast.classList.add('fading');
    setTimeout(() => {
      if (toast.parentElement) toast.remove();
    }, 250);
  }, 3500);
}

// =============================================================================
// 7. İndirme & e-Okul (< 20 KB) Sıkıştırma Motoru
// =============================================================================
async function exportCanvasToBlob(canvas, preset) {
  if (preset === 'eokul') {
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
    downloadCurrentPhoto();
  }
}
