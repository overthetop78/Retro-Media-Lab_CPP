# Primitives et attributs communs

`TrackEncodingAttributeKeys.h` reprend les six clés et la construction des tags du C#. Les constantes propres aux encodeurs Apple II, Apple IWM, Commodore et ISO FM/MFM pointent maintenant vers ces définitions communes. Le contrat reste indépendant de MediaFileSystems et du GUI.

L'écriture 24 bits little endian est centralisée dans `MediaFileSystems::Binary::writeLittle24`. La destination est vérifiée avant toute modification. Le formateur ADFS et le codec ProDOS l'utilisent ; ProDOS conserve sa validation de plage propre au format avant cet appel. Les trois octets de poids faible sont conservés, comme les conversions d'octets du C#.

Une compilation native du bloc a réussi. Un seul contrôle ciblé a vérifié les clés, les tags, l'ordre des octets, la troncature à 24 bits, les limites de destination dont SIZE_MAX et l'absence d'écriture partielle sur refus. Il n'a décodé ni encodé aucun média. Le contrôle temporaire a été supprimé.

Les mentions anciennes d'encodeurs restants ont été rapprochées des sources existantes : `Track.cpp` valide les requêtes et fournit la construction ISO commune ; `Composition/FluxCodecs.cpp` enregistre les encodeurs ; `FmEncoder.cpp` et `MfmEncoder.cpp` utilisent les attributs physiques et le diagnostic de capacité. Les codes de taille ISO sont bornés par `SectorConstants` et `SizeCodes.cpp`. Ces algorithmes n'ont pas été réécrits dans ce bloc.
