# Bibliothèques d’archives intégrées

Sources récupérées le 14 novembre 2026 ; révisions conservées localement dans third_party/. Les tests et outils externes ne sont pas compilés.

- libarchive : [4a5944b726d24c8e86589ad09cef958c6f1a6e9c](https://github.com/libarchive/libarchive/tree/4a5944b726d24c8e86589ad09cef958c6f1a6e9c).
- zlib : [99623ef7097352da7993ba614161fd6a311e046e](https://github.com/madler/zlib/tree/99623ef7097352da7993ba614161fd6a311e046e).
- xz : [f54febc540607b3bdee0a0e6793355df2a7e9f61](https://github.com/tukaani-project/xz/tree/f54febc540607b3bdee0a0e6793355df2a7e9f61).
- zstd : [9d728bb08622963fa95fbb0c788ac13162829916](https://github.com/facebook/zstd/tree/9d728bb08622963fa95fbb0c788ac13162829916).
- lz4 : [08cb1a67e30d938961cb5f6c284595165d7c5a33](https://github.com/lz4/lz4/tree/08cb1a67e30d938961cb5f6c284595165d7c5a33).
- BZip2 1.0.8 : https://sourceware.org/pub/bzip2/bzip2-1.0.8.tar.gz ; SHA-256 ab5a03176ee106d3f0fa90e381da478ddae405918153cca248e682cd0c4a2269.

libarchive fournit l’API multiformat. zlib, BZip2, liblzma (XZ), Zstandard et LZ4 fournissent les codecs. Ces dépendances sont liées statiquement et leurs licences sont distribuées avec RML. ZIP, 7z, RAR/RAR5 et TAR sont traités par libarchive ; la disponibilité de chaque filtre est vérifiée par le banc de validation. RAR/RAR5 sont en lecture, pas en création.
