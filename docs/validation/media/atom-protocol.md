# Protocole de bande Atom

Références C# inchangées : `Images/Reading/Decoding/Sequential/Acorn/AtomTapeDecoder.cs` et `Images/Writing/Encoding/Sequential/Acorn/AtomTapeEncoder.cs` dans `F:/GW GUI`. SHA vérifiés dans l’inventaire.

Le décodeur respecte les formats et la machine acceptés par le C#. TAP utilise directement le codec ; UEF série compatible est parcouru sans synthèse ; les autres cycles UEF sont synthétisés en PCM avant le FSK. Un document PCM déjà matérialisé gardant le format UEF suit directement le chemin PCM. Les sources WAV/CSW/RAW/VOC utilisent le premier canal natif et le FSK 300 bauds, 1200/2400 Hz et 8N1 existant. Les diagnostics et les segments non résolus sont conservés, y compris lorsque les recettes traduisibles remplacent les chaînes C#.

Le rattachement des positions source distinctes suit leur ordre d’apparition. La méthode withSourcePositions partage les octets immuables du bloc, au lieu de les recopier. Les positions négatives restent refusées. La sélection de l’encodeur valide aussi les noms, adresses et intégrité via le codec commun sans créer de tampon TAP complet ; la production réelle reste unique.

L’encodeur reconstruit les blocs TAP/UEF avec le lecteur Atom existant, ou synthétise le PCM à la cadence demandée avec les fonctions FSK partagées. Il conserve la borne de 128 millions d’échantillons et la métadonnée reconstructedSignal pour le PCM. Les segments retenus et les fichiers non valides sont refusés ; aucun moteur de secours n’est choisi silencieusement.

Compilation Debug réussie sans avertissement après ajout de l’inclusion explicite du helper d’identifiants dans le portage. Contrôle ciblé PASS via les registres : TAP, UEF série, écriture/relecture UEF, UEF cycles, PCM FSK et UEF déjà matérialisé, normalisation RAW vers WAV, partage des données lors du rattachement, métadonnée de reconstruction, rejet machine/profil/limite, conservation des checksums invalides et des framing incompatibles, annulation. Le fichier décodé est reconstitué à l’identique. Aucun média utilisateur ; banc temporaire et artefacts supprimés après consignation.

Les quatre diagnostics sont générés avec Argos dans les 29 langues, relus/corrigés et compilés dans les 30 DLL de catalogues. Audit PASS : 1271 fichiers C++, 56 clés d’erreur App et 29 langues.

La composition par défaut globale, les services de conversion/ouverture et leur raccordement à App restent à porter. Le contrôle de registres explicitement composés ici ne prouve pas leur composition automatique.
