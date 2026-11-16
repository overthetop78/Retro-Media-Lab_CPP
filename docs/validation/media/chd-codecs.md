# Validation ciblée des codecs CHD

Le 16 novembre 2026, MediaEngine compile en Debug sous MSVC 19.44 (`build/chd-codecs-final-build.log`). Le programme temporaire a réussi : `PASS CHD MSB/Huffman/DEFLATE/LZMA/Zstandard/FLAC predictors-CRC-stereo/consumption/cancellation` (`build/chd-codecs-smoke.log`). Il a ensuite été supprimé avec ses fichiers et binaires.

Les cas ciblés couvrent une lecture MSB non alignée de 32 bits, les deux imports Huffman, les flux DEFLATE sur plusieurs chunks et avec fin suivie d’octets, LZMA avec/sans marqueur terminal et refus des octets supplémentaires, Zstandard concaténé avec trame skippable et troncature. Les trames FLAC synthétiques ont des CRC recalculés indépendamment et couvrent sous-trames constantes/brutes/fixed/LPC, canaux indépendants/left-side/right-side/mid-side, endian, consommation, CRC erronés, troncature et annulation.

Les 19 erreurs sont générées avec Argos dans les 29 langues puis relues ; les contresens sur répétition Huffman, troncature, code unaire, prédicteur, résidus et CRC sont corrigés. Les 30 DLL Constants/langues compilent (`build/chd-codecs-catalogs.log`). L’audit des littéraux passe sur 1115 fichiers C++ (`build/chd-codecs-audit.log`). Aucun média utilisateur ni ancienne suite n’a été utilisé.

Ces contrôles portent sur les codecs communs ; ils ne prouvent pas encore la lecture d’un conteneur CHD depuis App.
