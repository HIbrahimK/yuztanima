// YüzGizle PWA Service Worker (Çevrimdışı Önbellekleme)
const CACHE_NAME = 'yuzgizle-pwa-v1.3.1';

const ASSETS_TO_CACHE = [
  './',
  './index.html',
  './manifest.json',
  './css/style.css',
  './js/app.js',
  './js/ort.min.js',
  './js/ort-wasm-simd.wasm',
  './js/ort-wasm.wasm',
  './js/tf.min.js',
  './js/blazeface.min.js',
  './js/jszip.min.js',
  './models/face_detection_yunet_2023mar.onnx',
  './models/model.json',
  './models/group1-shard1of1.bin',
  './icons/icon-192.png',
  './icons/icon-512.png',
  './emojis/smile.png',
  './emojis/cool.png',
  './emojis/grin.png',
  './emojis/party.png',
  './emojis/mask.png',
  './emojis/star.png',
  './emojis/angel.png',
  './emojis/cat.png',
  './emojis/dog.png',
  './emojis/panda.png',
  './emojis/bear.png',
  './emojis/lion.png',
  './emojis/fox.png',
  './emojis/robot.png',
  './emojis/flower.png',
  './emojis/sunglasses.png',
  './emojis/grad.png',
  './emojis/heart.png',
  './emojis/rocket.png',
  './emojis/slight_smile.png'
];

// Localhost veya yerel IP kontrolü (Geliştirme ortamında önbelleğe takılmayı engeller)
const isLocalhost = Boolean(
  self.location.hostname === 'localhost' ||
  self.location.hostname === '[::1]' ||
  self.location.hostname.match(/^127(?:\.(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)){3}$/)
);

// Kurulum ve önbellekleme
self.addEventListener('install', (event) => {
  self.skipWaiting();
  event.waitUntil(
    caches.open(CACHE_NAME).then((cache) => {
      console.log('[ServiceWorker] Çevrimdışı varlıklar önbelleğe alınıyor...');
      return cache.addAll(ASSETS_TO_CACHE);
    })
  );
});

// Eski önbellekleri temizleme
self.addEventListener('activate', (event) => {
  event.waitUntil(
    caches.keys().then((cacheNames) => {
      return Promise.all(
        cacheNames.map((cache) => {
          if (cache !== CACHE_NAME) {
            console.log('[ServiceWorker] Eski önbellek temizleniyor:', cache);
            return caches.delete(cache);
          }
        })
      );
    }).then(() => self.clients.claim())
  );
});

// İstekleri sunma (Network-First for Code/Localhost, Cache-First for Heavy Assets)
self.addEventListener('fetch', (event) => {
  if (event.request.method !== 'GET') return;

  const url = new URL(event.request.url);
  const isHtmlOrCode = event.request.mode === 'navigate' ||
                       url.pathname.endsWith('.html') ||
                       url.pathname.endsWith('.js') ||
                       url.pathname.endsWith('.css') ||
                       url.pathname === '/' ||
                       url.pathname.endsWith('/');

  // Ctrl+F5 yapıldığında (request.cache === 'reload'), Localhost'ta veya HTML/JS/CSS güncellemelerinde:
  // Önce Ağı Dene (Network-First); ağ yoksa çevrimdışı önbellekten sun.
  if (event.request.cache === 'reload' || isLocalhost || isHtmlOrCode) {
    event.respondWith(
      fetch(event.request)
        .then((networkResponse) => {
          if (networkResponse && networkResponse.status === 200) {
            const responseToCache = networkResponse.clone();
            caches.open(CACHE_NAME).then((cache) => cache.put(event.request, responseToCache));
          }
          return networkResponse;
        })
        .catch(() => {
          // İnternet / sunucu kapalıysa (çevrimdışı kullanım): Önbellekten sun
          return caches.match(event.request).then((cached) => {
            if (cached) return cached;
            if (event.request.mode === 'navigate' || (event.request.headers.get('accept') && event.request.headers.get('accept').includes('text/html'))) {
              return caches.match('./index.html');
            }
          });
        })
    );
    return;
  }

  // Büyük statik dosyalar (WASM, ONNX, model ağırlıkları) için Cache-First (Hızlı açılış)
  event.respondWith(
    caches.match(event.request).then((cachedResponse) => {
      if (cachedResponse) {
        return cachedResponse;
      }
      return fetch(event.request).then((networkResponse) => {
        if (networkResponse && networkResponse.status === 200) {
          const responseToCache = networkResponse.clone();
          caches.open(CACHE_NAME).then((cache) => {
            cache.put(event.request, responseToCache);
          });
        }
        return networkResponse;
      });
    })
  );
});
