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
  activeTool: 'add', // 'add' (Ekle), 'remove' (Çıkar), 'exempt' (Muafiyet), 'pan' (Kaydır)
  selectedFaceIndex: -1, // Seçili alanın indeksi (-1 = seçili yok)
  undoStack: [], // Geri alma geçmişi

  // Tuval Yakınlaştırma & Gezinme (Zoom & Pan)
  zoom: 1.0, // 0.25 - 4.0
  panX: 0,
  panY: 0,
  isPanning: false,
  panStartX: 0,
  panStartY: 0,

  // Alan Boyutlandırma & Taşıma (Resize & Move)
  isResizing: false,
  activeResizeHandle: null, // 'tl', 'tr', 'bl', 'br'
  resizeStartPos: { x: 0, y: 0 },
  resizeOriginalFace: null,
  isMovingFace: false,
  moveStartPos: { x: 0, y: 0 },
  moveOriginalFace: null,

  // Ayarlar
  method: 'blur', // blur, pixelate, blackbar, emoji
  blurStrength: 30,
  pixelSize: 18,
  selectedEmoji: 'smile.png',
  
  // Yapay Zeka Modeli & Algılama Motoru (YuNet ONNX vs BlazeFace)
  engineType: 'yunet', // 'yunet' (YuNet ONNX - Birincil & Masaüstü Modeli) | 'blazeface' (TFJS)
  yunetSession: null,
  yunetLoading: false,

  // Yüz Algılama Ayarları (Derin Izgara Taraması & Hassasiyet)
  scanMode: 'deep', // 'deep' (Derin Tarama - Izgara / Kalabalık Sınıf) | 'fast' (Hızlı - Portre)
  scanSensitivity: 2, // 1: Dengeli (0.46), 2: Yüksek (0.35), 3: Maksimum (0.24)
  
  preset: 'original', // original (Varsayılan: Kırpma yok, tam görünüm), meb_manset, eokul, mebbis, insta_square, insta_story
  
  // Filigran & Logo (JPG ve PNG Desteği, Serbest Konumlandırma)
  watermarkEnabled: false,
  watermarkText: 'T.C. Millî Eğitim Bakanlığı',
  watermarkOpacity: 0.85,
  logoImg: null,
  logoScale: 0.18, // 0.05 - 0.45
  logoXRatio: 0.88, // 0.0 - 1.0 (X konumu)
  logoYRatio: 0.88, // 0.0 - 1.0 (Y konumu)
  isMovingLogo: false,
  logoMoveStart: { x: 0, y: 0, initXRatio: 0.88, initYRatio: 0.88 },

  // Çizim & Dokunmatik Etkileşim
  isPointerDown: false,
  dragStartCanvasX: 0,
  dragStartCanvasY: 0,
  dragCurrentCanvasX: 0,
  dragCurrentCanvasY: 0
};

// 20 Çeşit Emojiyi Hızlı ve Çevrimdışı Kullanım İçin Önceden Yükle
const emojiCache = {};
const EMOJI_NAMES = [
  'smile.png', 'cool.png', 'grin.png', 'party.png', 'mask.png',
  'star.png', 'angel.png', 'cat.png', 'dog.png', 'panda.png',
  'bear.png', 'lion.png', 'fox.png', 'robot.png', 'flower.png',
  'sunglasses.png', 'grad.png', 'heart.png', 'rocket.png', 'slight_smile.png'
];
EMOJI_NAMES.forEach(name => {
  const img = new Image();
  img.src = `emojis/${name}`;
  emojiCache[name] = img;
});

// =============================================================================
// 1. PWA & Model Başlatma
// =============================================================================
document.addEventListener('DOMContentLoaded', async () => {
  initPWA();
  initTabs();
  initEventListeners();
  updateScanBadge();
  // Birincil yöntem olan YuNet ONNX modelini yükle
  await loadYuNetModel();
  // Alternatif model BlazeFace'i arka planda hazırla
  loadFaceModel().catch(e => console.log('BlazeFace arka plan yükleme:', e));
});

function initPWA() {
  if (window.location.protocol === 'file:') {
    const warning = document.getElementById('fileProtocolWarning');
    if (warning) warning.style.display = 'flex';
    console.warn('⚠️ file:// protokolü algılandı. Tarayıcı güvenlik (CORS) kısıtlaması nedeniyle Service Worker ve yapay zeka modelleri sadece http://localhost:8080 gibi bir yerel sunucu üzerinden çalışır.');
    return;
  }

  if ('serviceWorker' in navigator) {
    navigator.serviceWorker.register('./sw.js')
      .then(reg => {
        console.log('PWA ServiceWorker kayıtlı:', reg.scope);
        // Sayfa her açıldığında arka planda yeni sürüm kontrolü yap
        reg.update();
        reg.onupdatefound = () => {
          const installingWorker = reg.installing;
          if (installingWorker) {
            installingWorker.onstatechange = () => {
              if (installingWorker.state === 'installed' && navigator.serviceWorker.controller) {
                console.log('Yeni PWA sürümü algılandı, sayfa yenileniyor...');
                window.location.reload();
              }
            };
          }
        };
      })
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

async function loadYuNetModel() {
  if (state.yunetSession) return state.yunetSession;
  if (state.yunetLoading) {
    while (state.yunetLoading) {
      await new Promise(r => setTimeout(r, 80));
    }
    return state.yunetSession;
  }

  state.yunetLoading = true;
  const loading = document.getElementById('loadingOverlay');
  const loadingText = document.getElementById('loadingText');
  try {
    if (loading && state.images.length > 0) {
      loading.style.display = 'flex';
      if (loadingText) loadingText.textContent = '🧠 YuNet yapay zeka modeli çevrimdışı yükleniyor...';
    }

    if (typeof window.ort === 'undefined') {
      throw new Error('ONNX Runtime Web kütüphanesi (ort.min.js) bulunamadı.');
    }

    // WASM ikili dosya yolları
    window.ort.env.wasm.wasmPaths = './js/';
    window.ort.env.wasm.numThreads = Math.min(4, navigator.hardwareConcurrency || 2);

    // ONNX modelini ArrayBuffer olarak indir ve session oluştur
    const resp = await fetch('./models/face_detection_yunet_2023mar.onnx');
    if (!resp.ok) throw new Error(`YuNet modeli indirilemedi: HTTP ${resp.status}`);
    const modelBuffer = await resp.arrayBuffer();

    state.yunetSession = await window.ort.InferenceSession.create(modelBuffer, {
      executionProviders: ['wasm']
    });

    console.log('✅ YuNet ONNX modeli başarıyla yüklendi (WIDER FACE DNN - Masaüstü Modeli).');
    return state.yunetSession;
  } catch (error) {
    console.warn('YuNet ONNX yüklenirken hata oluştu:', error);
    state.yunetSession = null;
    return null;
  } finally {
    state.yunetLoading = false;
    if (loading && state.images.length === 0) loading.style.display = 'none';
  }
}

async function loadFaceModel() {
  const loading = document.getElementById('loadingOverlay');
  const loadingText = document.getElementById('loadingText');
  try {
    state.modelLoading = true;
    if (loading && state.engineType === 'blazeface') {
      loading.style.display = 'flex';
      loadingText.textContent = 'BlazeFace modeli çevrimdışı yükleniyor...';
    }

    if (window.blazeface) {
      // Yerel model dosyasından yükle (Harici sunucu bağımlılığı yok)
      state.model = await blazeface.load({
        modelUrl: './models/model.json',
        maxFaces: 100,
        scoreThreshold: 0.42,
        iouThreshold: 0.3
      });
      console.log('✅ Yerel BlazeFace modeli başarıyla yüklendi (maxFaces: 100).');
    }
  } catch (error) {
    console.warn('Yerel BlazeFace model yüklenirken hata, CDN deneniyor:', error);
    try {
      if (window.blazeface) {
        state.model = await blazeface.load({
          maxFaces: 100,
          scoreThreshold: 0.42,
          iouThreshold: 0.3
        });
      }
    } catch (err2) {
      console.error('BlazeFace modeli yüklenemedi:', err2);
    }
  } finally {
    state.modelLoading = false;
    if (loading && state.images.length === 0) loading.style.display = 'none';
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

  // Filigran & Logo (JPG, PNG, WebP)
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
        const file = e.target.files[0];
        const reader = new FileReader();
        reader.onload = (ev) => {
          const img = new Image();
          img.onload = () => {
            state.logoImg = img;
            const fn = document.getElementById('logoFileName');
            if (fn) {
              fn.style.display = 'block';
              fn.textContent = `✅ Yüklendi: ${file.name} (${img.width}×${img.height})`;
            }
            const btnRem = document.getElementById('btnRemoveLogo');
            if (btnRem) btnRem.style.display = 'inline-block';
            const scaleGrp = document.getElementById('logoScaleGroup');
            if (scaleGrp) scaleGrp.style.display = 'flex';
            renderCurrentCanvas();
            showToast('✅ Logo/Resim yüklendi. İstediğiniz yere taşıyabilirsiniz.');
          };
          img.src = ev.target.result;
        };
        reader.readAsDataURL(file);
      }
    });
  }

  // Logo Kaldır Butonu
  const btnRemoveLogo = document.getElementById('btnRemoveLogo');
  if (btnRemoveLogo) {
    btnRemoveLogo.addEventListener('click', () => {
      state.logoImg = null;
      btnRemoveLogo.style.display = 'none';
      const fn = document.getElementById('logoFileName');
      if (fn) fn.style.display = 'none';
      const scaleGrp = document.getElementById('logoScaleGroup');
      if (scaleGrp) scaleGrp.style.display = 'none';
      renderCurrentCanvas();
      showToast('🗑️ Okul logosu kaldırıldı.');
    });
  }

  // Saydamlık Slider'ı
  const watermarkOpacitySlider = document.getElementById('watermarkOpacitySlider');
  const watermarkOpacityVal = document.getElementById('watermarkOpacityVal');
  if (watermarkOpacitySlider) {
    watermarkOpacitySlider.addEventListener('input', (e) => {
      const val = parseInt(e.target.value);
      state.watermarkOpacity = val / 100;
      if (watermarkOpacityVal) watermarkOpacityVal.textContent = `%${val}`;
      renderCurrentCanvas();
    });
  }

  // Logo Boyut Slider'ı
  const logoScaleSlider = document.getElementById('logoScaleSlider');
  const logoScaleVal = document.getElementById('logoScaleVal');
  if (logoScaleSlider) {
    logoScaleSlider.addEventListener('input', (e) => {
      const val = parseInt(e.target.value);
      state.logoScale = val / 100;
      if (logoScaleVal) logoScaleVal.textContent = `%${val}`;
      renderCurrentCanvas();
    });
  }

  // Logo Konum Slider'ları ve Önayarları
  const logoXSlider = document.getElementById('logoXSlider');
  if (logoXSlider) {
    logoXSlider.addEventListener('input', (e) => {
      state.logoXRatio = parseInt(e.target.value) / 100;
      document.querySelectorAll('.pos-btn').forEach(b => b.classList.remove('active'));
      renderCurrentCanvas();
    });
  }

  const logoYSlider = document.getElementById('logoYSlider');
  if (logoYSlider) {
    logoYSlider.addEventListener('input', (e) => {
      state.logoYRatio = parseInt(e.target.value) / 100;
      document.querySelectorAll('.pos-btn').forEach(b => b.classList.remove('active'));
      renderCurrentCanvas();
    });
  }

  document.querySelectorAll('.pos-btn[data-pos]').forEach(btn => {
    btn.addEventListener('click', () => {
      document.querySelectorAll('.pos-btn').forEach(b => b.classList.remove('active'));
      btn.classList.add('active');
      const pos = btn.getAttribute('data-pos');
      if (pos === 'top-left') {
        state.logoXRatio = 0.05;
        state.logoYRatio = 0.05;
      } else if (pos === 'top-right') {
        state.logoXRatio = 0.95;
        state.logoYRatio = 0.05;
      } else if (pos === 'center') {
        state.logoXRatio = 0.50;
        state.logoYRatio = 0.50;
      } else if (pos === 'bottom-left') {
        state.logoXRatio = 0.05;
        state.logoYRatio = 0.95;
      } else if (pos === 'bottom-right') {
        state.logoXRatio = 0.88;
        state.logoYRatio = 0.88;
      }
      if (logoXSlider) logoXSlider.value = Math.round(state.logoXRatio * 100);
      if (logoYSlider) logoYSlider.value = Math.round(state.logoYRatio * 100);
      renderCurrentCanvas();
    });
  });

  // Yakınlaştırma, Uzaklaştırma ve Sığdır Kontrolleri
  const btnZoomIn = document.getElementById('btnZoomIn');
  const btnZoomOut = document.getElementById('btnZoomOut');
  const btnZoomFit = document.getElementById('btnZoomFit');
  const zoomPercentBadge = document.getElementById('zoomPercentBadge');
  if (btnZoomIn) btnZoomIn.addEventListener('click', () => zoomIn(0.25));
  if (btnZoomOut) btnZoomOut.addEventListener('click', () => zoomOut(0.25));
  if (btnZoomFit) btnZoomFit.addEventListener('click', resetZoomFit);
  if (zoomPercentBadge) zoomPercentBadge.addEventListener('click', resetZoomFit);

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
  const btnToolPan = document.getElementById('btnToolPan');
  const btnUndo = document.getElementById('btnUndo');

  if (btnToolAdd) btnToolAdd.addEventListener('click', () => setActiveTool('add'));
  if (btnToolRemove) {
    btnToolRemove.addEventListener('click', () => {
      if (state.selectedFaceIndex >= 0) {
        deleteSelectedFace();
      } else {
        setActiveTool('remove');
      }
    });
  }
  if (btnToolExempt) btnToolExempt.addEventListener('click', () => setActiveTool('exempt'));
  if (btnToolPan) btnToolPan.addEventListener('click', () => setActiveTool('pan'));
  if (btnUndo) btnUndo.addEventListener('click', undoLastAction);

  // Seçili Alan Boyutlandırma Butonları (+10%, -10%)
  const btnGrowSelectedFace = document.getElementById('btnGrowSelectedFace');
  const btnShrinkSelectedFace = document.getElementById('btnShrinkSelectedFace');
  if (btnGrowSelectedFace) btnGrowSelectedFace.addEventListener('click', () => growSelectedFace(1.10));
  if (btnShrinkSelectedFace) btnShrinkSelectedFace.addEventListener('click', () => shrinkSelectedFace(0.90));

  // Yüz Tanı / Tarama Butonları
  const btnScanFacesCanvas = document.getElementById('btnScanFacesCanvas');
  const btnScanFaces = document.getElementById('btnScanFaces');
  const btnScanCurrentTab = document.getElementById('btnScanCurrentTab');
  const btnScanAllTab = document.getElementById('btnScanAllTab');

  if (btnScanFacesCanvas) btnScanFacesCanvas.addEventListener('click', scanFacesCurrentPhoto);
  if (btnScanFaces) btnScanFaces.addEventListener('click', scanFacesCurrentPhoto);
  if (btnScanCurrentTab) btnScanCurrentTab.addEventListener('click', scanFacesCurrentPhoto);
  if (btnScanAllTab) btnScanAllTab.addEventListener('click', scanFacesAllPhotos);

  // Yapay Zeka Modeli Seçimi (YuNet ONNX vs BlazeFace)
  document.querySelectorAll('[data-engine-type]').forEach(card => {
    card.addEventListener('click', async () => {
      document.querySelectorAll('[data-engine-type]').forEach(c => c.classList.remove('selected'));
      card.classList.add('selected');
      state.engineType = card.getAttribute('data-engine-type');

      updateScanBadge();

      if (state.engineType === 'yunet' && !state.yunetSession) {
        await loadYuNetModel();
      } else if (state.engineType === 'blazeface' && !state.model) {
        await loadFaceModel();
      }

      const name = (state.engineType === 'yunet')
        ? '🧠 YuNet (ONNX - Masaüstü Kalitesi)'
        : '⚡ BlazeFace (TFJS - Web Modeli)';
      showToast(`${name} seçildi.`);
    });
  });

  // Tarama Modu Kartları (Derin vs Hızlı)
  document.querySelectorAll('[data-scan-mode]').forEach(card => {
    card.addEventListener('click', () => {
      document.querySelectorAll('[data-scan-mode]').forEach(c => c.classList.remove('selected'));
      card.classList.add('selected');
      state.scanMode = card.getAttribute('data-scan-mode');
      updateScanBadge();
    });
  });

  // Tarama Hassasiyeti Slider'ı
  const scanSensitivitySlider = document.getElementById('scanSensitivitySlider');
  const scanSensitivityVal = document.getElementById('scanSensitivityVal');
  if (scanSensitivitySlider) {
    scanSensitivitySlider.addEventListener('input', (e) => {
      state.scanSensitivity = parseInt(e.target.value);
      if (scanSensitivityVal) {
        if (state.scanSensitivity === 1) {
          scanSensitivityVal.textContent = 'Dengeli (%50)';
          scanSensitivityVal.style.color = '#94a3b8';
        } else if (state.scanSensitivity === 2) {
          scanSensitivityVal.textContent = 'Yüksek (%85) [Önerilen]';
          scanSensitivityVal.style.color = '#60a5fa';
        } else {
          scanSensitivityVal.textContent = 'Maksimum (%98) [Zor/Gölge]';
          scanSensitivityVal.style.color = '#ec4899';
        }
      }
    });
  }

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

  // Seçilen model henüz hazır değilse önce kesinlikle yüklenmesini bekle
  if (state.engineType === 'yunet' && !state.yunetSession) {
    if (loadingText) loadingText.textContent = '🧠 YuNet yapay zeka modeli hazırlanıyor...';
    await loadYuNetModel();
  } else if (state.engineType === 'blazeface' && !state.model) {
    if (loadingText) loadingText.textContent = '⚡ BlazeFace yapay zeka modeli hazırlanıyor...';
    await loadFaceModel();
  }

  const prevTotal = state.images.length;
  let addedCount = 0;

  for (let i = 0; i < files.length; i++) {
    const file = files[i];
    if (!file.type.startsWith('image/')) continue;

    if (loadingText) {
      const engineName = (state.engineType === 'yunet') ? '🧠 YuNet' : '⚡ BlazeFace';
      loadingText.textContent = `${engineName} ile yüzler taranıyor (${i + 1}/${files.length}): ${file.name}...`;
    }

    const img = await loadImageFromFile(file);
    if (!img) continue;

    // Yüz tespiti yap (Seçili model & mod & hassasiyet)
    const faces = await detectFacesInImage(img);

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

function updateScanBadge() {
  const badge = document.getElementById('scanStatusBadge');
  if (!badge) return;

  if (state.engineType === 'yunet') {
    badge.textContent = (state.scanMode === 'deep') ? '🧠 YuNet ONNX (Derin)' : '🧠 YuNet ONNX (Hızlı)';
    badge.style.background = 'rgba(16, 185, 129, 0.15)';
    badge.style.color = '#34d399';
    badge.style.borderColor = 'rgba(16, 185, 129, 0.3)';
  } else {
    badge.textContent = (state.scanMode === 'deep') ? '⚡ BlazeFace (Derin)' : '⚡ BlazeFace (Hızlı)';
    badge.style.background = 'rgba(59, 130, 246, 0.15)';
    badge.style.color = '#60a5fa';
    badge.style.borderColor = 'rgba(59, 130, 246, 0.3)';
  }
}

function getSensitivityThreshold() {
  if (state.scanSensitivity === 1) return 0.46; // Dengeli
  if (state.scanSensitivity === 3) return 0.24; // Maksimum
  return 0.35; // Yüksek (Varsayılan)
}

/**
 * YuNet için 640x640 Letterbox BGR Float32 Tensor Üretici
 */
function createYuNetTensor(source, targetDim = 640) {
  const canvas = document.createElement('canvas');
  canvas.width = targetDim;
  canvas.height = targetDim;
  const ctx = canvas.getContext('2d');

  ctx.fillStyle = '#000000';
  ctx.fillRect(0, 0, targetDim, targetDim);

  const srcW = source.width;
  const srcH = source.height;
  const scale = Math.min(targetDim / srcW, targetDim / srcH);
  const scaledW = Math.round(srcW * scale);
  const scaledH = Math.round(srcH * scale);
  const padX = Math.round((targetDim - scaledW) / 2);
  const padY = Math.round((targetDim - scaledH) / 2);

  ctx.drawImage(source, 0, 0, srcW, srcH, padX, padY, scaledW, scaledH);
  const imgData = ctx.getImageData(0, 0, targetDim, targetDim);
  const data = imgData.data;

  // YuNet BGR formatında unnormalized float32 [1, 3, targetDim, targetDim]
  const planeSize = targetDim * targetDim;
  const floatArr = new Float32Array(3 * planeSize);

  for (let y = 0; y < targetDim; y++) {
    for (let x = 0; x < targetDim; x++) {
      const srcIdx = (y * targetDim + x) * 4;
      const r = data[srcIdx];
      const g = data[srcIdx + 1];
      const b = data[srcIdx + 2];

      const offset = y * targetDim + x;
      floatArr[0 * planeSize + offset] = b; // B
      floatArr[1 * planeSize + offset] = g; // G
      floatArr[2 * planeSize + offset] = r; // R
    }
  }

  const tensor = new window.ort.Tensor('float32', floatArr, [1, 3, targetDim, targetDim]);
  return { tensor, scale, padX, padY, targetDim };
}

/**
 * YuNet Çıkışlarını (cls, obj, bbox) Çözen ve Koordinatları Dönüştüren Fonksiyon
 */
function decodeYuNetOutputs(outputs, meta, scoreThreshold) {
  const strides = [8, 16, 32];
  const candidates = [];
  const { scale, padX, padY, targetDim } = meta;

  for (let s = 0; s < strides.length; s++) {
    const stride = strides[s];
    const cols = targetDim / stride;
    const rows = targetDim / stride;

    const cls = outputs['cls_' + stride].data;
    const obj = outputs['obj_' + stride].data;
    const bbox = outputs['bbox_' + stride].data;

    for (let r = 0; r < rows; r++) {
      for (let c = 0; c < cols; c++) {
        const idx = r * cols + c;
        const cls_score = Math.min(1, Math.max(0, cls[idx]));
        const obj_score = Math.min(1, Math.max(0, obj[idx]));
        const score = Math.sqrt(cls_score * obj_score);

        if (score < scoreThreshold) continue;

        // YuNet bounding box formülü (OpenCV spesifikasyonu)
        const cx = (c + bbox[idx * 4 + 0]) * stride;
        const cy = (r + bbox[idx * 4 + 1]) * stride;
        const w = Math.exp(bbox[idx * 4 + 2]) * stride;
        const h = Math.exp(bbox[idx * 4 + 3]) * stride;

        const x1 = cx - w / 2;
        const y1 = cy - h / 2;

        const origX1 = (x1 - padX) / scale;
        const origY1 = (y1 - padY) / scale;
        const origX2 = (x1 + w - padX) / scale;
        const origY2 = (y1 + h - padY) / scale;

        candidates.push({
          x1: origX1,
          y1: origY1,
          x2: origX2,
          y2: origY2,
          w: origX2 - origX1,
          h: origY2 - origY1,
          score: score
        });
      }
    }
  }

  return candidates;
}

/**
 * YuNet ONNX Yüz Algılama Motoru (Masaüstü OpenCV ile %100 Birebir Model)
 */
async function detectFacesWithYuNet(img, customThreshold = null) {
  if (!state.yunetSession) {
    await loadYuNetModel();
  }
  if (!state.yunetSession) {
    console.warn('YuNet oturumu oluşturulamadı, BlazeFace deneniyor...');
    return detectFacesWithBlazeFace(img, customThreshold);
  }

  // YuNet hassasiyet eşikleri
  let threshold = customThreshold;
  if (threshold === null) {
    if (state.scanSensitivity === 1) threshold = 0.55;
    else if (state.scanSensitivity === 3) threshold = 0.28;
    else threshold = 0.40; // Yüksek (Önerilen)
  }

  const allCandidates = [];

  // 1. AŞAMA: TAM GÖRÜNTÜ LETTERBOX 640x640 GEÇİŞİ (Global Pass)
  try {
    const meta = createYuNetTensor(img, 640);
    const outputs = await state.yunetSession.run({ input: meta.tensor });
    const globalCands = decodeYuNetOutputs(outputs, meta, threshold);
    allCandidates.push(...globalCands);
  } catch (err) {
    console.warn('YuNet global geçiş hatası:', err);
  }

  // 2. AŞAMA: ÇOK ÖLÇEKLİ PARÇALI IZGARA TARAMASI (Derin Tarama / Kalabalık Sınıf)
  if (state.scanMode === 'deep' && (img.width >= 800 || img.height >= 800)) {
    const aspect = img.width / img.height;
    let cols = (aspect >= 1.5) ? 3 : 2;
    let rows = (aspect <= 0.75) ? 3 : 2;

    const overlap = 0.25;
    const tileW = Math.round(img.width / (cols - (cols - 1) * overlap));
    const tileH = Math.round(img.height / (rows - (rows - 1) * overlap));
    const stepX = Math.round(tileW * (1 - overlap));
    const stepY = Math.round(tileH * (1 - overlap));

    const tileCanvas = document.createElement('canvas');
    tileCanvas.width = tileW;
    tileCanvas.height = tileH;
    const tileCtx = tileCanvas.getContext('2d');

    for (let r = 0; r < rows; r++) {
      for (let c = 0; c < cols; c++) {
        const cropX = Math.min(img.width - tileW, c * stepX);
        const cropY = Math.min(img.height - tileH, r * stepY);

        tileCtx.clearRect(0, 0, tileW, tileH);
        tileCtx.drawImage(img, cropX, cropY, tileW, tileH, 0, 0, tileW, tileH);

        try {
          const tileMeta = createYuNetTensor(tileCanvas, 640);
          const tileOutputs = await state.yunetSession.run({ input: tileMeta.tensor });
          const tileCands = decodeYuNetOutputs(tileOutputs, tileMeta, threshold);

          for (const cand of tileCands) {
            allCandidates.push({
              x1: cropX + cand.x1,
              y1: cropY + cand.y1,
              x2: cropX + cand.x2,
              y2: cropY + cand.y2,
              w: cand.w,
              h: cand.h,
              score: cand.score
            });
          }
        } catch (tErr) {
          console.warn(`YuNet tile [${r}, ${c}] hatası:`, tErr);
        }
      }
    }
  }

  // 3. AŞAMA: NON-MAXIMUM SUPPRESSION (NMS) & BİRLEŞTİRME
  const mergedBoxes = applyNms(allCandidates, 0.35);

  console.log(`🧠 YuNet ile ${mergedBoxes.length} yüz başarıyla tespit edildi (Adaylar: ${allCandidates.length}).`);

  // 4. AŞAMA: SAÇ & ÇENE GÜVENLİK PAYI
  return mergedBoxes.map(box => {
    const padX = box.w * 0.12;
    const padY = box.h * 0.18;

    const finalX = Math.max(0, box.x1 - padX);
    const finalY = Math.max(0, box.y1 - padY);
    const finalW = Math.min(img.width - finalX, box.w + (padX * 2));
    const finalH = Math.min(img.height - finalY, box.h + (padY * 2));

    return {
      x: Math.round(finalX),
      y: Math.round(finalY),
      width: Math.round(finalW),
      height: Math.round(finalH),
      exempt: false,
      manual: false,
      score: box.score
    };
  });
}

/**
 * BlazeFace (TFJS) Çok Ölçekli Parçalı Yüz Algılama Motoru (Alternatif Hafif Model)
 */
async function detectFacesWithBlazeFace(img, customThreshold = null) {
  if (!state.model) {
    console.log('BlazeFace hazır değil, yükleniyor...');
    await loadFaceModel();
  }
  if (!state.model) {
    console.warn('BlazeFace yüklenemedi.');
    return [];
  }

  const threshold = (customThreshold !== null) ? customThreshold : getSensitivityThreshold();
  const width = img.width;
  const height = img.height;
  const allCandidates = [];

  try {
    state.model.scoreThreshold = threshold;
    state.model.maxFaces = 100;

    const maxDim = 1280;
    let globalCanvas = document.createElement('canvas');
    let globalScale = 1.0;

    if (width > maxDim || height > maxDim) {
      globalScale = maxDim / Math.max(width, height);
      globalCanvas.width = Math.round(width * globalScale);
      globalCanvas.height = Math.round(height * globalScale);
    } else {
      globalCanvas.width = width;
      globalCanvas.height = height;
    }

    const globalCtx = globalCanvas.getContext('2d');
    globalCtx.drawImage(img, 0, 0, globalCanvas.width, globalCanvas.height);

    let globalPreds = await state.model.estimateFaces(globalCanvas, false);

    if ((!globalPreds || globalPreds.length === 0) && threshold > 0.28) {
      state.model.scoreThreshold = 0.26;
      globalPreds = await state.model.estimateFaces(globalCanvas, false);
      state.model.scoreThreshold = threshold;
    }

    if (globalPreds && globalPreds.length > 0) {
      for (const p of globalPreds) {
        const gx1 = p.topLeft[0] / globalScale;
        const gy1 = p.topLeft[1] / globalScale;
        const gx2 = p.bottomRight[0] / globalScale;
        const gy2 = p.bottomRight[1] / globalScale;
        const score = (Array.isArray(p.probability) ? p.probability[0] : p.probability) || 0.85;

        allCandidates.push({
          x1: gx1,
          y1: gy1,
          x2: gx2,
          y2: gy2,
          w: gx2 - gx1,
          h: gy2 - gy1,
          score: score
        });
      }
    }

    if (state.scanMode === 'deep' && width >= 380 && height >= 260) {
      const aspect = width / height;
      let cols = (aspect >= 1.6) ? 4 : 3;
      let rows = (aspect <= 0.75) ? 3 : 2;

      const overlap = 0.28;
      const tileW = Math.round(width / (cols - (cols - 1) * overlap));
      const tileH = Math.round(height / (rows - (rows - 1) * overlap));
      const stepX = Math.round(tileW * (1 - overlap));
      const stepY = Math.round(tileH * (1 - overlap));

      const tileCanvas = document.createElement('canvas');
      const tileCtx = tileCanvas.getContext('2d');

      const maxTileDim = 640;
      const tileScale = Math.min(1.0, maxTileDim / Math.max(tileW, tileH));
      tileCanvas.width = Math.round(tileW * tileScale);
      tileCanvas.height = Math.round(tileH * tileScale);

      for (let r = 0; r < rows; r++) {
        for (let c = 0; c < cols; c++) {
          const cropX = Math.min(width - tileW, c * stepX);
          const cropY = Math.min(height - tileH, r * stepY);

          tileCtx.clearRect(0, 0, tileCanvas.width, tileCanvas.height);
          tileCtx.drawImage(img, cropX, cropY, tileW, tileH, 0, 0, tileCanvas.width, tileCanvas.height);

          try {
            const tilePreds = await state.model.estimateFaces(tileCanvas, false);
            if (tilePreds && tilePreds.length > 0) {
              for (const p of tilePreds) {
                const tx1 = cropX + (p.topLeft[0] / tileScale);
                const ty1 = cropY + (p.topLeft[1] / tileScale);
                const tx2 = cropX + (p.bottomRight[0] / tileScale);
                const ty2 = cropY + (p.bottomRight[1] / tileScale);
                const score = (Array.isArray(p.probability) ? p.probability[0] : p.probability) || 0.8;

                allCandidates.push({
                  x1: tx1,
                  y1: ty1,
                  x2: tx2,
                  y2: ty2,
                  w: tx2 - tx1,
                  h: ty2 - ty1,
                  score: score
                });
              }
            }
          } catch (tErr) {
            console.warn(`BlazeFace tile [${r},${c}] hatası:`, tErr);
          }
        }
      }
    }

    const mergedBoxes = applyNms(allCandidates, 0.35);

    return mergedBoxes.map(box => {
      const padX = box.w * 0.12;
      const padY = box.h * 0.18;

      const finalX = Math.max(0, box.x1 - padX);
      const finalY = Math.max(0, box.y1 - padY);
      const finalW = Math.min(width - finalX, box.w + (padX * 2));
      const finalH = Math.min(height - finalY, box.h + (padY * 2));

      return {
        x: Math.round(finalX),
        y: Math.round(finalY),
        width: Math.round(finalW),
        height: Math.round(finalH),
        exempt: false,
        manual: false,
        score: box.score
      };
    });
  } catch (err) {
    console.error('BlazeFace tespiti sırasında hata:', err);
    return [];
  }
}

/**
 * Ana Yüz Tespiti Giriş Noktası
 * Aktif seçili olan modele (YuNet ONNX veya BlazeFace) yönlendirir.
 */
async function detectFacesInImage(img, customThreshold = null) {
  if (state.engineType === 'yunet') {
    try {
      const faces = await detectFacesWithYuNet(img, customThreshold);
      if (faces && faces.length > 0) return faces;
      // YuNet ile yüz bulunamadıysa BlazeFace ile teyit et
      console.log('YuNet 0 yüz buldu, BlazeFace ile teyit ediliyor...');
      const fallback = await detectFacesWithBlazeFace(img, customThreshold);
      if (fallback && fallback.length > 0) return fallback;
      return faces || [];
    } catch (err) {
      console.warn('YuNet çalıştırılamadı, BlazeFace yedek motoruna geçiliyor:', err);
      return detectFacesWithBlazeFace(img, customThreshold);
    }
  } else {
    return detectFacesWithBlazeFace(img, customThreshold);
  }
}

/**
 * Non-Maximum Suppression (NMS)
 * Çakışan aday yüz kutularını skora göre eler ve temizler
 */
function applyNms(boxes, iouThreshold = 0.35) {
  if (!boxes || boxes.length === 0) return [];

  // Skora göre azalan sırala
  const sorted = [...boxes].sort((a, b) => b.score - a.score);
  const selected = [];

  for (const current of sorted) {
    if (current.w <= 8 || current.h <= 8) continue; // Minik gürültüleri filtrele

    let shouldKeep = true;
    for (const existing of selected) {
      const xA = Math.max(current.x1, existing.x1);
      const yA = Math.max(current.y1, existing.y1);
      const xB = Math.min(current.x2, existing.x2);
      const yB = Math.min(current.y2, existing.y2);

      const interWidth = Math.max(0, xB - xA);
      const interHeight = Math.max(0, yB - yA);
      const interArea = interWidth * interHeight;

      if (interArea > 0) {
        const areaA = current.w * current.h;
        const areaB = existing.w * existing.h;
        const unionArea = areaA + areaB - interArea;
        const iou = interArea / unionArea;
        const containment = interArea / Math.min(areaA, areaB);

        // Ya %35'ten fazla örtüşüyor ya da biri diğerinin %65 içine gömülmüş
        if (iou >= iouThreshold || containment >= 0.65) {
          shouldKeep = false;
          break;
        }
      }
    }

    if (shouldKeep) {
      selected.push(current);
    }
  }

  return selected;
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
  resetZoomFit();
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

/**
 * Verilen bir tuvale fotoğrafı ve gizleme efektlerini (ve isteğe bağlı filigranı) çizer.
 * @param {HTMLCanvasElement} canvas - Çizim yapılacak hedef tuval
 * @param {Object} current - Çizilecek görsel öğesi ({ img, faces, name, ... })
 * @param {boolean} isExport - true ise kılavuz kenarlıklar, silme rozetleri, seçim çizgileri ve tutamaçlar ASLA ÇİZİLMEZ!
 */
function renderPhotoToCanvas(canvas, current, isExport = false) {
  if (!current || !current.img) return;

  const ctx = canvas.getContext('2d');
  const transform = getCanvasTransform(current, canvas);

  // 1. Resmi Çiz
  ctx.drawImage(current.img, transform.sx, transform.sy, transform.sw, transform.sh, 0, 0, canvas.width, canvas.height);

  // 2. Yüzleri Gizle (Rehber çerçeveler YALNIZCA isExport === false iken ekranda çizilir)
  current.faces.forEach((face, idx) => {
    const cRect = imageToCanvasRect(face, transform, canvas);

    if (!face.exempt) {
      applyHidingStrategy(ctx, cRect.x, cRect.y, cRect.width, cRect.height);
    }

    // Sadece UI önizleme modunda mavi/sarı/turuncu çerçeveler, silme butonları ve tutamaçlar çizilir
    // İndirilen nihai fotoğrafta ASLA ÇİZİLMEZ!
    if (!isExport) {
      const isSelected = (idx === state.selectedFaceIndex);
      drawFaceGuide(ctx, cRect.x, cRect.y, cRect.width, cRect.height, face, isSelected, state.activeTool, idx);
    }
  });

  // 3. Kullanıcı Parmağıyla/Fareyle Sürükleme Yapıyorsa Canlı Seçim Kutusunu Çiz (Yalnızca ekran önizleme)
  if (!isExport && state.isPointerDown) {
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

  // 4. Logo ve Filigran (Hem ekranda hem de indirilen fotoğrafta uygulanır)
  if (state.watermarkEnabled || state.logoImg) {
    applyWatermark(ctx, canvas.width, canvas.height);
  }

  // 5. Durum Hapını Güncelle (Yalnızca ekran önizleme modunda)
  if (!isExport) {
    updateStatsPill(current.faces);
  }
}

function renderCurrentCanvas() {
  const current = state.images[state.currentIndex];
  if (!current) return;

  const canvas = document.getElementById('previewCanvas');
  renderPhotoToCanvas(canvas, current, false);
}

/**
 * İndirme ve dışa aktarma (ZIP, Paylaşım) için kılavuzsuz, tertemiz bir tuval üretir.
 */
function createExportCanvas(current) {
  const exportCanvas = document.createElement('canvas');
  renderPhotoToCanvas(exportCanvas, current, true);
  return exportCanvas;
}

function calculatePresetDimensions(origW, origH, preset) {
  switch (preset) {
    case 'original':
      return { width: origW, height: origH };
    case 'meb_manset':
      return { width: 1200, height: 568 }; // 19:9 MEB Standardı
    case 'eokul':
      return { width: 133, height: 171 }; // e-Okul Standart
    case 'mebbis':
      return { width: 394, height: 512 }; // MEBBİS
    case 'insta_square':
      return { width: 1080, height: 1080 };
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
    let emojiImg = emojiCache[state.selectedEmoji];
    if (!emojiImg) {
      emojiImg = new Image();
      emojiImg.src = `emojis/${state.selectedEmoji}`;
      emojiCache[state.selectedEmoji] = emojiImg;
    }
    if (emojiImg.complete && emojiImg.naturalWidth > 0) {
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

    // 4 Köşe Tutamacı (Geniş & Dokunmatik Dostu Dairesel Tutamaçlar)
    const handleRadius = Math.max(6, Math.min(10, Math.round(8 / (state.zoom || 1))));
    ctx.setLineDash([]);
    const corners = [
      [x, y],
      [x + w, y],
      [x, y + h],
      [x + w, y + h]
    ];
    corners.forEach(([cx, cy]) => {
      ctx.beginPath();
      ctx.arc(cx, cy, handleRadius, 0, Math.PI * 2);
      ctx.fillStyle = '#ffffff';
      ctx.fill();
      ctx.lineWidth = 2.5;
      ctx.strokeStyle = '#0284c7';
      ctx.stroke();
    });

    // Başlık Rozeti ve Boyut Bilgisi
    ctx.fillStyle = 'rgba(14, 165, 233, 0.95)';
    ctx.beginPath();
    ctx.roundRect(x, Math.max(0, y - 24), 148, 22, 4);
    ctx.fill();
    ctx.fillStyle = '#ffffff';
    ctx.font = 'bold 11px sans-serif';
    ctx.fillText(`🎯 Alan #${faceIdx + 1} (↔ Boyutlandır)`, x + 6, Math.max(15, y - 9));
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
  if (!state.watermarkEnabled && !state.logoImg) return;

  ctx.save();
  ctx.globalAlpha = state.watermarkOpacity;

  let logoW = 0;
  let logoH = 0;
  let logoX = 0;
  let logoY = 0;

  if (state.logoImg) {
    logoW = Math.max(20, Math.floor(canvasW * (state.logoScale || 0.18)));
    logoH = Math.round((state.logoImg.height / state.logoImg.width) * logoW);
    logoX = Math.round((canvasW - logoW) * state.logoXRatio);
    logoY = Math.round((canvasH - logoH) * state.logoYRatio);
    logoX = Math.max(0, Math.min(canvasW - logoW, logoX));
    logoY = Math.max(0, Math.min(canvasH - logoH, logoY));

    ctx.drawImage(state.logoImg, logoX, logoY, logoW, logoH);
  }

  if (state.watermarkEnabled && state.watermarkText && state.watermarkText.trim() !== '') {
    const fontSize = Math.max(14, Math.floor(canvasW * 0.024));
    ctx.font = `bold ${fontSize}px sans-serif`;
    const textMetrics = ctx.measureText(state.watermarkText);
    const textW = textMetrics.width;
    const textH = fontSize * 1.4;

    let textX, textY;
    if (state.logoImg) {
      textX = Math.round(logoX + (logoW - textW) / 2);
      textY = (logoY + logoH + fontSize + 4 > canvasH) ? (logoY - 8) : (logoY + logoH + fontSize + 4);
      textX = Math.max(8, Math.min(canvasW - textW - 8, textX));
    } else {
      textX = Math.round((canvasW - textW - 16) * state.logoXRatio) + 8;
      textY = Math.round((canvasH - textH) * state.logoYRatio) + fontSize;
      textX = Math.max(8, Math.min(canvasW - textW - 8, textX));
      textY = Math.max(fontSize + 6, Math.min(canvasH - 8, textY));
    }

    ctx.fillStyle = 'rgba(15, 23, 42, 0.75)';
    ctx.beginPath();
    ctx.roundRect(textX - 8, textY - fontSize - 4, textW + 16, textH, 6);
    ctx.fill();

    ctx.fillStyle = '#ffffff';
    ctx.fillText(state.watermarkText, textX, textY);
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
// Yakınlaştırma (Zoom), Kaydırma (Pan) ve Boyutlandırma Yardımcıları
// =============================================================================

function applyCanvasTransform() {
  const canvas = document.getElementById('previewCanvas');
  if (!canvas) return;
  canvas.style.transform = `translate(${state.panX}px, ${state.panY}px) scale(${state.zoom})`;
  const zoomBadge = document.getElementById('zoomPercentBadge');
  if (zoomBadge) {
    zoomBadge.textContent = `%${Math.round(state.zoom * 100)}`;
  }
}

function resetZoomFit() {
  state.zoom = 1.0;
  state.panX = 0;
  state.panY = 0;
  applyCanvasTransform();
}

function zoomIn(step = 0.25) {
  state.zoom = Math.min(4.0, +(state.zoom + step).toFixed(2));
  applyCanvasTransform();
}

function zoomOut(step = 0.25) {
  state.zoom = Math.max(0.25, +(state.zoom - step).toFixed(2));
  applyCanvasTransform();
}

function setZoom(val) {
  state.zoom = Math.max(0.25, Math.min(4.0, val));
  applyCanvasTransform();
}

function growSelectedFace(factor = 1.10) {
  const current = state.images[state.currentIndex];
  if (!current || state.selectedFaceIndex < 0 || !current.faces[state.selectedFaceIndex]) return;
  saveUndoSnapshot('Alanı Büyüt');
  const face = current.faces[state.selectedFaceIndex];
  const oldW = face.width;
  const oldH = face.height;
  const newW = Math.round(oldW * factor);
  const newH = Math.round(oldH * factor);
  face.x = Math.max(0, Math.round(face.x - (newW - oldW) / 2));
  face.y = Math.max(0, Math.round(face.y - (newH - oldH) / 2));
  face.width = newW;
  face.height = newH;
  renderCurrentCanvas();
  showToast('➕ Alan boyutu büyütüldü (%10).');
}

function shrinkSelectedFace(factor = 0.90) {
  const current = state.images[state.currentIndex];
  if (!current || state.selectedFaceIndex < 0 || !current.faces[state.selectedFaceIndex]) return;
  const face = current.faces[state.selectedFaceIndex];
  if (face.width < 20 || face.height < 20) {
    showToast('⚠️ Alan daha fazla küçültülemez.');
    return;
  }
  saveUndoSnapshot('Alanı Küçült');
  const oldW = face.width;
  const oldH = face.height;
  const newW = Math.max(16, Math.round(oldW * factor));
  const newH = Math.max(16, Math.round(oldH * factor));
  face.x = Math.max(0, Math.round(face.x + (oldW - newW) / 2));
  face.y = Math.max(0, Math.round(face.y + (oldH - newH) / 2));
  face.width = newW;
  face.height = newH;
  renderCurrentCanvas();
  showToast('➖ Alan boyutu küçültüldü (%10).');
}

function getHandleAt(pos, cRect, zoom = 1.0) {
  const hitRadius = Math.max(18, 28 / (zoom || 1));
  const handles = [
    { key: 'tl', x: cRect.x, y: cRect.y },
    { key: 'tr', x: cRect.x + cRect.width, y: cRect.y },
    { key: 'bl', x: cRect.x, y: cRect.y + cRect.height },
    { key: 'br', x: cRect.x + cRect.width, y: cRect.y + cRect.height }
  ];
  for (const h of handles) {
    if (Math.hypot(pos.x - h.x, pos.y - h.y) <= hitRadius) {
      return h.key;
    }
  }
  return null;
}

function isInsideRect(pos, rect) {
  return pos.x >= rect.x && pos.x <= (rect.x + rect.width) &&
         pos.y >= rect.y && pos.y <= (rect.y + rect.height);
}

// =============================================================================
// 6. Tuval Etkileşimi (Pixel-Perfect Koordinat Eşleme & Dokunmatik Zoom)
// =============================================================================
function setupCanvasInteractions() {
  const canvas = document.getElementById('previewCanvas');

  const getCanvasPixelCoords = (clientX, clientY) => {
    const rect = canvas.getBoundingClientRect();
    const scaleX = canvas.width / rect.width;
    const scaleY = canvas.height / rect.height;
    return {
      x: (clientX - rect.left) * scaleX,
      y: (clientY - rect.top) * scaleY
    };
  };

  let initialPinchDist = 0;
  let initialPinchZoom = 1.0;
  let initialPinchPanX = 0;
  let initialPinchPanY = 0;
  let initialPinchCenter = { x: 0, y: 0 };
  let isPinching = false;

  const onPointerDown = (clientX, clientY, button = 0, isShift = false, isSpace = false) => {
    if (isPinching) return;

    const current = state.images[state.currentIndex];
    if (!current) return;

    // 1. Pan (Kaydırma) Modu, Orta Tuş veya Boşluk Tuşu ile Kaydırma
    if (state.activeTool === 'pan' || button === 1 || isSpace) {
      state.isPanning = true;
      state.panStartX = clientX - state.panX;
      state.panStartY = clientY - state.panY;
      canvas.classList.add('cursor-panning');
      return;
    }

    const pos = getCanvasPixelCoords(clientX, clientY);
    const transform = getCanvasTransform(current, canvas);

    // 2. Logo Taşıma Kontrolü (Eğer logo yüklüyse ve logo üzerine tıklandıysa)
    if (state.logoImg) {
      const logoW = Math.max(20, Math.floor(canvas.width * (state.logoScale || 0.18)));
      const logoH = Math.round((state.logoImg.height / state.logoImg.width) * logoW);
      const logoX = Math.round((canvas.width - logoW) * state.logoXRatio);
      const logoY = Math.round((canvas.height - logoH) * state.logoYRatio);

      if (isInsideRect(pos, { x: logoX, y: logoY, width: logoW, height: logoH })) {
        state.isMovingLogo = true;
        state.logoMoveStart = {
          x: pos.x,
          y: pos.y,
          initXRatio: state.logoXRatio,
          initYRatio: state.logoYRatio
        };
        return;
      }
    }

    // 3. Seçili Alan Boyutlandırma Kontrolü (Köşe Tutamaçları)
    if (state.selectedFaceIndex >= 0 && current.faces[state.selectedFaceIndex]) {
      const selectedFace = current.faces[state.selectedFaceIndex];
      const cRect = imageToCanvasRect(selectedFace, transform, canvas);
      const handle = getHandleAt(pos, cRect, state.zoom);

      if (handle) {
        state.isResizing = true;
        state.activeResizeHandle = handle;
        state.resizeStartPos = { ...pos };
        state.resizeOriginalFace = { ...selectedFace };
        return;
      }

      // 4. Seçili Alanı Sürükleyerek Taşıma (Box Repositioning)
      if (state.activeTool !== 'remove' && isInsideRect(pos, cRect)) {
        state.isMovingFace = true;
        state.moveStartPos = { ...pos };
        state.moveOriginalFace = { ...selectedFace };
        return;
      }
    }

    // 5. Yeni Alan Çizimi veya Tıklama
    state.isPointerDown = true;
    state.dragStartCanvasX = pos.x;
    state.dragStartCanvasY = pos.y;
    state.dragCurrentCanvasX = pos.x;
    state.dragCurrentCanvasY = pos.y;
  };

  const onPointerMove = (clientX, clientY) => {
    // 1. Pan yapılıyorsa
    if (state.isPanning) {
      state.panX = clientX - state.panStartX;
      state.panY = clientY - state.panStartY;
      applyCanvasTransform();
      return;
    }

    const current = state.images[state.currentIndex];
    if (!current) return;

    const pos = getCanvasPixelCoords(clientX, clientY);
    const transform = getCanvasTransform(current, canvas);

    // 2. Logo Taşınıyorsa
    if (state.isMovingLogo && state.logoImg) {
      const logoW = Math.max(20, Math.floor(canvas.width * (state.logoScale || 0.18)));
      const logoH = Math.round((state.logoImg.height / state.logoImg.width) * logoW);
      const spanX = Math.max(1, canvas.width - logoW);
      const spanY = Math.max(1, canvas.height - logoH);

      const dx = (pos.x - state.logoMoveStart.x) / spanX;
      const dy = (pos.y - state.logoMoveStart.y) / spanY;

      state.logoXRatio = Math.max(0, Math.min(1, +(state.logoMoveStart.initXRatio + dx).toFixed(3)));
      state.logoYRatio = Math.max(0, Math.min(1, +(state.logoMoveStart.initYRatio + dy).toFixed(3)));

      const sX = document.getElementById('logoXSlider');
      const sY = document.getElementById('logoYSlider');
      if (sX) sX.value = Math.round(state.logoXRatio * 100);
      if (sY) sY.value = Math.round(state.logoYRatio * 100);

      renderCurrentCanvas();
      return;
    }

    // 3. Seçili Alan Boyutlandırılıyorsa (Resize)
    if (state.isResizing && state.selectedFaceIndex >= 0 && current.faces[state.selectedFaceIndex]) {
      const face = current.faces[state.selectedFaceIndex];
      const invScaleX = transform.sw / canvas.width;
      const invScaleY = transform.sh / canvas.height;
      const dx = (pos.x - state.resizeStartPos.x) * invScaleX;
      const dy = (pos.y - state.resizeStartPos.y) * invScaleY;
      const orig = state.resizeOriginalFace;

      let newX = orig.x;
      let newY = orig.y;
      let newW = orig.width;
      let newH = orig.height;

      if (state.activeResizeHandle === 'br') {
        newW = Math.max(16, orig.width + dx);
        newH = Math.max(16, orig.height + dy);
      } else if (state.activeResizeHandle === 'tr') {
        newW = Math.max(16, orig.width + dx);
        const targetH = orig.height - dy;
        if (targetH >= 16) {
          newY = orig.y + dy;
          newH = targetH;
        }
      } else if (state.activeResizeHandle === 'bl') {
        const targetW = orig.width - dx;
        if (targetW >= 16) {
          newX = orig.x + dx;
          newW = targetW;
        }
        newH = Math.max(16, orig.height + dy);
      } else if (state.activeResizeHandle === 'tl') {
        const targetW = orig.width - dx;
        const targetH = orig.height - dy;
        if (targetW >= 16) {
          newX = orig.x + dx;
          newW = targetW;
        }
        if (targetH >= 16) {
          newY = orig.y + dy;
          newH = targetH;
        }
      }

      face.x = Math.round(newX);
      face.y = Math.round(newY);
      face.width = Math.round(newW);
      face.height = Math.round(newH);
      renderCurrentCanvas();
      return;
    }

    // 4. Seçili Alan Taşınıyorsa (Move)
    if (state.isMovingFace && state.selectedFaceIndex >= 0 && current.faces[state.selectedFaceIndex]) {
      const face = current.faces[state.selectedFaceIndex];
      const invScaleX = transform.sw / canvas.width;
      const invScaleY = transform.sh / canvas.height;
      const dx = (pos.x - state.moveStartPos.x) * invScaleX;
      const dy = (pos.y - state.moveStartPos.y) * invScaleY;
      const orig = state.moveOriginalFace;

      face.x = Math.max(0, Math.round(orig.x + dx));
      face.y = Math.max(0, Math.round(orig.y + dy));
      renderCurrentCanvas();
      return;
    }

    // 5. Yeni Alan Çiziliyorsa
    if (state.isPointerDown) {
      state.dragCurrentCanvasX = pos.x;
      state.dragCurrentCanvasY = pos.y;
      renderCurrentCanvas();
    }
  };

  const onPointerUp = (clientX, clientY, isShiftHeld = false) => {
    if (state.isPanning) {
      state.isPanning = false;
      canvas.classList.remove('cursor-panning');
      return;
    }

    if (state.isMovingLogo) {
      state.isMovingLogo = false;
      showToast('📍 Logo konumu güncellendi.');
      renderCurrentCanvas();
      return;
    }

    if (state.isResizing) {
      state.isResizing = false;
      state.activeResizeHandle = null;
      saveUndoSnapshot('Alan Boyutlandırıldı');
      showToast('📐 Alan boyutu güncellendi.');
      renderCurrentCanvas();
      return;
    }

    if (state.isMovingFace) {
      state.isMovingFace = false;
      saveUndoSnapshot('Alan Taşındı');
      showToast('✥ Alan yeni konuma taşındı.');
      renderCurrentCanvas();
      return;
    }

    if (!state.isPointerDown) return;
    state.isPointerDown = false;

    const current = state.images[state.currentIndex];
    if (!current) return;

    const pos = getCanvasPixelCoords(clientX, clientY);
    const dist = Math.hypot(pos.x - state.dragStartCanvasX, pos.y - state.dragStartCanvasY);
    const transform = getCanvasTransform(current, canvas);

    if (dist < 15) {
      // 1. TIKLAMA: Bir yüzün veya alanın üzerine mi tıklandı?
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
        if (isShiftHeld || state.activeTool === 'remove') {
          deleteFace(clickedIdx);
        } else if (state.activeTool === 'exempt') {
          toggleFaceExempt(clickedIdx);
        } else {
          if (state.selectedFaceIndex === clickedIdx) {
            toggleFaceExempt(clickedIdx);
          } else {
            selectFace(clickedIdx);
          }
        }
      } else {
        deselectFace();
      }
      return;
    } else if (dist >= 18) {
      // 2. SÜRÜKLEME: Yeni Manuel Alan Ekle
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
            exempt: (state.activeTool === 'exempt'),
            manual: true
          });

          selectFace(current.faces.length - 1);
          showToast('➕ Yeni alan eklendi. Köşelerden boyutlandırabilirsiniz.');
          renderCurrentCanvas();
        }
      }
    }

    renderCurrentCanvas();
  };

  // Fare Etkinlikleri
  canvas.addEventListener('mousedown', (e) => {
    onPointerDown(e.clientX, e.clientY, e.button, e.shiftKey, e.code === 'Space');
  });
  window.addEventListener('mousemove', (e) => {
    onPointerMove(e.clientX, e.clientY);
  });
  window.addEventListener('mouseup', (e) => {
    onPointerUp(e.clientX, e.clientY, e.shiftKey);
  });

  // Fare Tekerleği ile Yakınlaştırma (Wheel Zoom)
  canvas.addEventListener('wheel', (e) => {
    e.preventDefault();
    const delta = e.deltaY < 0 ? 0.15 : -0.15;
    setZoom(Math.max(0.25, Math.min(4.0, +(state.zoom + delta).toFixed(2))));
  }, { passive: false });

  // Dokunmatik Etkinlikler (Mobil ve Tablet Uyumlu + Pinch to Zoom)
  canvas.addEventListener('touchstart', (e) => {
    if (e.touches.length === 2) {
      isPinching = true;
      state.isPointerDown = false;
      state.isResizing = false;
      state.isMovingFace = false;
      state.isMovingLogo = false;
      const t1 = e.touches[0];
      const t2 = e.touches[1];
      initialPinchDist = Math.hypot(t1.clientX - t2.clientX, t1.clientY - t2.clientY);
      initialPinchZoom = state.zoom;
      initialPinchPanX = state.panX;
      initialPinchPanY = state.panY;
      initialPinchCenter = {
        x: (t1.clientX + t2.clientX) / 2,
        y: (t1.clientY + t2.clientY) / 2
      };
      e.preventDefault();
      return;
    }

    if (e.touches.length === 1) {
      isPinching = false;
      onPointerDown(e.touches[0].clientX, e.touches[0].clientY, 0, false, false);
    }
  }, { passive: false });

  canvas.addEventListener('touchmove', (e) => {
    if (e.touches.length === 2 && isPinching) {
      const t1 = e.touches[0];
      const t2 = e.touches[1];
      const curDist = Math.hypot(t1.clientX - t2.clientX, t1.clientY - t2.clientY);
      if (initialPinchDist > 5) {
        const factor = curDist / initialPinchDist;
        state.zoom = Math.max(0.25, Math.min(4.0, +(initialPinchZoom * factor).toFixed(2)));
        const curCenter = {
          x: (t1.clientX + t2.clientX) / 2,
          y: (t1.clientY + t2.clientY) / 2
        };
        state.panX = initialPinchPanX + (curCenter.x - initialPinchCenter.x);
        state.panY = initialPinchPanY + (curCenter.y - initialPinchCenter.y);
        applyCanvasTransform();
      }
      e.preventDefault();
      return;
    }

    if (e.touches.length === 1 && !isPinching) {
      onPointerMove(e.touches[0].clientX, e.touches[0].clientY);
      if (state.isPointerDown || state.isPanning || state.isResizing || state.isMovingFace || state.isMovingLogo) {
        e.preventDefault();
      }
    }
  }, { passive: false });

  canvas.addEventListener('touchend', (e) => {
    if (isPinching && e.touches.length < 2) {
      isPinching = false;
      return;
    }
    if (e.changedTouches.length === 1 && !isPinching) {
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
    const engineLabel = (state.engineType === 'yunet') ? '🧠 YuNet' : '⚡ BlazeFace';
    loadingText.textContent = `${engineLabel} ile yüzler yeniden taranıyor...`;
  }

  // Model hazır değilse yükle
  if (state.engineType === 'yunet' && !state.yunetSession) {
    await loadYuNetModel();
  } else if (state.engineType === 'blazeface' && !state.model) {
    await loadFaceModel();
  }

  saveUndoSnapshot('Yüzleri Yeniden Tara');

  // Kullanıcının elle eklediği manuel gizleme alanlarını koru
  const existingManual = current.faces.filter(f => f.manual);

  // Yüzleri tara
  const newDetectedFaces = await detectFacesInImage(current.img);

  current.faces = [...newDetectedFaces, ...existingManual];
  state.selectedFaceIndex = -1;
  updateSelectedFaceBar();
  renderThumbnails();
  renderCurrentCanvas();

  if (loading) loading.style.display = 'none';

  if (navigator.vibrate) navigator.vibrate([40, 30, 40]);

  if (newDetectedFaces.length > 0) {
    const engineLabel = (state.engineType === 'yunet') ? '🧠 YuNet (Masaüstü Kalitesi)' : '⚡ BlazeFace';
    showToast(`✅ ${engineLabel} ile ${newDetectedFaces.length} yüz bulundu.`, 'Geri Al', undoLastAction);
  } else {
    showToast('ℹ️ Fotoğrafta otomatik yüz bulunamadı. "Algılama Hassasiyeti"ni artırabilir veya elle ekleyebilirsiniz.');
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

  if (state.engineType === 'yunet' && !state.yunetSession) {
    if (loadingText) loadingText.textContent = '🧠 YuNet modeli hazırlanıyor...';
    await loadYuNetModel();
  } else if (state.engineType === 'blazeface' && !state.model) {
    if (loadingText) loadingText.textContent = '⚡ BlazeFace modeli hazırlanıyor...';
    await loadFaceModel();
  }

  saveUndoSnapshot('Tüm Fotoğraflar Yeniden Tarandı');

  let totalFound = 0;
  for (let i = 0; i < state.images.length; i++) {
    const item = state.images[i];
    if (loadingText) {
      const engineLabel = (state.engineType === 'yunet') ? '🧠 YuNet' : '⚡ BlazeFace';
      loadingText.textContent = `${engineLabel} (${i + 1}/${state.images.length}): ${item.name}...`;
    }

    const existingManual = item.faces.filter(f => f.manual);
    const newFaces = await detectFacesInImage(item.img);
    item.faces = [...newFaces, ...existingManual];
    totalFound += newFaces.length;
  }

  if (loading) loading.style.display = 'none';
  renderThumbnails();
  renderCurrentCanvas();
  const engineLabel = (state.engineType === 'yunet') ? '🧠 YuNet' : '⚡ BlazeFace';
  showToast(`✅ ${engineLabel} ile tüm fotoğraflar tarandı: Toplam ${totalFound} yüz tespit edildi.`, 'Geri Al', undoLastAction);
}

// =============================================================================
// Dokunmatik Araçlar, Seçim, Çıkarma & Geri Alma Motoru
// =============================================================================

function setActiveTool(tool) {
  state.activeTool = tool;

  const btnAdd = document.getElementById('btnToolAdd');
  const btnRemove = document.getElementById('btnToolRemove');
  const btnExempt = document.getElementById('btnToolExempt');
  const btnPan = document.getElementById('btnToolPan');

  if (btnAdd) btnAdd.classList.toggle('active', tool === 'add');
  if (btnRemove) btnRemove.classList.toggle('active', tool === 'remove');
  if (btnExempt) btnExempt.classList.toggle('active', tool === 'exempt');
  if (btnPan) btnPan.classList.toggle('active', tool === 'pan');

  const canvas = document.getElementById('previewCanvas');
  if (canvas) {
    canvas.classList.toggle('cursor-pan', tool === 'pan');
  }

  const banner = document.getElementById('canvasModeHint');
  const hintIcon = document.getElementById('hintIcon');
  const hintText = document.getElementById('hintText');

  if (banner && hintText && hintIcon) {
    banner.className = `canvas-mode-banner mode-${tool}`;
    if (tool === 'add') {
      hintIcon.textContent = '💡';
      hintText.innerHTML = '<b>Ekleme Modu:</b> Parmağınızla tuval üzerinde sürükleyerek yeni alan çizin. Seçili kutuları köşelerinden boyutlandırabilirsiniz.';
    } else if (tool === 'remove') {
      hintIcon.textContent = '🗑️';
      hintText.innerHTML = '<b>Çıkarma Modu:</b> Silmek istediğiniz yüzün veya alanın üzerine dokunun.';
    } else if (tool === 'exempt') {
      hintIcon.textContent = '🛡️';
      hintText.innerHTML = '<b>Muafiyet Modu:</b> Öğretmen vb. gizlenmeyecek yüzün üzerine dokunun.';
    } else if (tool === 'pan') {
      hintIcon.textContent = '🖐️';
      hintText.innerHTML = '<b>Kaydırma Modu:</b> Tuvali parmağınızla veya fareyle istediğiniz yöne kaydırın.';
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
  } else if (e.key === '4' || e.key === 'h' || e.key === 'H') {
    setActiveTool('pan');
  } else if (e.key === '+' || e.key === '=') {
    e.preventDefault();
    zoomIn();
  } else if (e.key === '-' || e.key === '_') {
    e.preventDefault();
    zoomOut();
  } else if (e.key === '0') {
    e.preventDefault();
    resetZoomFit();
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

  // İndirme işlemi için kılavuzsuz ve etiketsiz temiz tuval oluşturulur
  const exportCanvas = createExportCanvas(current);
  const blob = await exportCanvasToBlob(exportCanvas, state.preset);

  const url = URL.createObjectURL(blob);
  const a = document.createElement('a');
  a.href = url;
  const baseName = current.name.substring(0, current.name.lastIndexOf('.')) || 'foto';
  a.download = `${baseName}_${state.preset}_yuzgizle.jpg`;
  document.body.appendChild(a);
  a.click();
  document.body.removeChild(a);
  URL.revokeObjectURL(url);

  showToast('✅ Fotoğraf temiz olarak indirildi (kılavuzlar ve etiketler kaldırıldı).');
}

async function downloadAllPhotosZip() {
  if (!window.JSZip || state.images.length === 0) return;

  const loading = document.getElementById('loadingOverlay');
  const loadingText = document.getElementById('loadingText');
  loading.style.display = 'flex';
  loadingText.textContent = 'Tüm fotoğraflar işlenip ZIP arşivi hazırlanıyor...';

  const zip = new JSZip();
  const folder = zip.folder('YuzGizle_Fotograflar');

  for (let i = 0; i < state.images.length; i++) {
    const item = state.images[i];
    loadingText.textContent = `İşleniyor (${i + 1}/${state.images.length}): ${item.name}`;
    // Her fotoğraf için kılavuzsuz ve etiketsiz temiz tuval oluşturulur
    const exportCanvas = createExportCanvas(item);
    const blob = await exportCanvasToBlob(exportCanvas, state.preset);
    const baseName = item.name.substring(0, item.name.lastIndexOf('.')) || `foto_${i + 1}`;
    folder.file(`${baseName}_${state.preset}.jpg`, blob);
  }

  loadingText.textContent = 'ZIP dosyası sıkıştırılıyor...';

  const content = await zip.generateAsync({ type: 'blob' });
  const a = document.createElement('a');
  a.href = URL.createObjectURL(content);
  a.download = `YuzGizle_Toplu_${Date.now()}.zip`;
  document.body.appendChild(a);
  a.click();
  document.body.removeChild(a);

  loading.style.display = 'none';
  showToast(`✅ ${state.images.length} fotoğraf temiz olarak ZIP dosyasına aktarıldı.`);
}

async function shareCurrentPhoto() {
  const current = state.images[state.currentIndex];
  if (!current) return;

  // Paylaşım için kılavuzsuz ve etiketsiz temiz tuval oluşturulur
  const exportCanvas = createExportCanvas(current);
  const blob = await exportCanvasToBlob(exportCanvas, state.preset);
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
