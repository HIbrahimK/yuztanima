// YüzGizle PWA Service Worker (Çevrimdışı Önbellekleme)
const CACHE_NAME = 'yuzgizle-pwa-v1.0.1';

const ASSETS_TO_CACHE = [
  './',
  './index.html',
  './manifest.json',
  './css/style.css',
  './js/app.js',
  './js/tf.min.js',
  './js/blazeface.min.js',
  './js/jszip.min.js',
  './models/model.json',
  './models/group1-shard1of1.bin',
  './icons/icon-192.png',
  './icons/icon-512.png',
  './emojis/smile.png',
  './emojis/cool.png',
  './emojis/grin.png',
  './emojis/mask.png',
  './emojis/star.png'
];

// Kurulum ve önbellekleme
self.addEventListener('install', (event) => {
  event.waitUntil(
    caches.open(CACHE_NAME).then((cache) => {
      console.log('[ServiceWorker] Çevrimdışı varlıklar önbelleğe alınıyor...');
      return cache.addAll(ASSETS_TO_CACHE);
    }).then(() => self.skipWaiting())
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

// İstekleri önbellekten sunma (Offline First)
self.addEventListener('fetch', (event) => {
  // Sadece HTTP/HTTPS GET isteklerini işle
  if (event.request.method !== 'GET') return;

  event.respondWith(
    caches.match(event.request).then((cachedResponse) => {
      if (cachedResponse) {
        return cachedResponse;
      }
      return fetch(event.request).then((networkResponse) => {
        // Geçerli yanıt ise ve yerel veya CDN ise önbelleğe al
        if (networkResponse && networkResponse.status === 200) {
          const responseToCache = networkResponse.clone();
          caches.open(CACHE_NAME).then((cache) => {
            cache.put(event.request, responseToCache);
          });
        }
        return networkResponse;
      }).catch(() => {
        // Çevrimdışı ve önbellekte yoksa index.html dön
        if (event.request.headers.get('accept').includes('text/html')) {
          return caches.match('./index.html');
        }
      });
    })
  );
});
