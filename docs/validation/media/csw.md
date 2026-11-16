# CSW

Référence : `F:/GW GUI/src/RetroMediaLab.MediaEngine/Images/Formats/Tape/Csw/CswReader.cs` et `CswWriter.cs` (sources inchangées, SHA vérifiés dans l’inventaire).

Le lecteur accepte CSW 1 et 2, RLE, et Z-RLE pour la version 2. Il conserve cadence et polarité en PCM mono signé 16 bits à amplitude ±16000. Le plafond de 128 millions d’échantillons du C# est conservé ; en-têtes, extensions, longueurs, compte d’impulsions et flux zlib sont validés. La décompression consomme les octets d’impulsion par blocs, sans tampon d’expansion intermédiaire complet. zlib est libéré par RAII sur réussite, exception et annulation.

Le writer reprend CSW 2 Z-RLE, le premier canal PCM natif, les durées des changements de signe et le maintien de la polarité lors des zéros. CSW conserve les transitions et leur cadence, pas l’amplitude analogique. L’écriture est atomique via le composant existant.

Compilation Debug réussie sans avertissement. Contrôle ciblé PASS : versions 1/2, extension de header, RLE/Z-RLE, polarités, impulsions longues, nulles, tronquées ou excessives, compte incohérent, premier canal stéréo, signal vide, aller-retour et annulation avec destination conservée. Le contrôle temporaire accepte les exceptions d’annulation des couches moteur et MFS ; cette adaptation concerne le banc uniquement. Aucun média utilisateur utilisé.

Les 29 catalogues ont été générés avec Argos puis relus. Les contresens, négations et termes techniques altérés sont corrigés ; CSW et zlib sont protégés dans la liste commune. Les 30 DLL de catalogues sont compilées. Audit PASS : 1245 fichiers C++, 56 clés d’erreur App, 29 langues.

Les artefacts temporaires du contrôle sont supprimés. La composition automatique et les services globaux de conversion restent à porter ; ce contrôle ne prouve pas leur fonctionnement depuis App.
