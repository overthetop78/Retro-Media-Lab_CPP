# Préparation CUE pour les consommateurs

Le bloc CUE de MediaRuntimePreparationService est isolé dans CuePreparation : CueDocument est un document public opaque et immuable. Il expose les fichiers déclarés, le contrôle d’une disposition binaire unique AUDIO/MODE1/2352 numérotée et indexée, le remplacement du premier fichier et l’écriture dans un flux emprunté. Les parseurs, modèles et sérialiseurs BinCue existants restent les seuls responsables des commandes et métadonnées. Aucun type MediaFileSystems n’apparaît dans la nouvelle API.

prepareCueWaveFilesAsync possède ses arguments et le callback de préparation jusqu’à la fin de la tâche. Le propriétaire du répertoire fournit stageFile, qui copie ses pistes et conserve la responsabilité de ses copies. Les WAVE PCM stéréo 16 bits à 44,1 kHz sont lus par wavePcmSource existant, sans décodage supplémentaire ni rééchantillonnage. Les octets PCM sont copiés exactement puis complétés par des zéros au secteur audio de 2 352 octets. Chaque destination répétée n’est convertie qu’une fois ; les déclarations et métadonnées restent présentes dans le CUE.

Les chemins des pistes sont réécrits relativement au CUE préparé, avec un chemin absolu lorsque les racines ne permettent pas un relatif. Une destination identique à une source ou au CUE préparé est refusée. Les fichiers convertis et le CUE sont publiés ensemble avec writeAtomicImageSet ; une erreur restaure les sorties déjà remplacées et supprime les temporaires du service. Les copies créées par stageFile restent sous la responsabilité du propriétaire, y compris si son callback échoue. Les sources ne sont pas modifiées par le service.

## Défaut C# corrigé

Le C# place le contrôle des modes WAVE dans la condition converted.Add(staged). Une deuxième déclaration du même WAVE ayant une piste non audio évite donc ce contrôle. Le C++ valide toutes les déclarations WAVE avant de dédupliquer leur conversion. Le diagnostic spécifique fourni par le consommateur est conservé ; sans callback, une erreur CUE déjà traduite est produite. Les erreurs filesystem natives sont transformées en diagnostic d’ouverture traduit.

## Vérification

Compilation Debug et contrôle ciblé réussis : CUE opaque, disposition et nombre maximal de pistes, renommage sans mutation du document initial, métadonnées, flux conservé ouvert et erreur de flux traduite. Un WAVE temporaire avec un nom accentué contient 589 frames stéréo : ses 2 356 octets PCM sont conservés exactement et la sortie fait 4 704 octets après remplissage. Deux déclarations du même WAVE conservent leurs références, sans deuxième conversion.

Refus confirmé du deuxième WAVE déclaré avec MODE1/2352, avec conservation du diagnostic métier du consommateur. Annulation depuis stageFile, refus d’une source comme destination et refus d’un tampon nul vérifiés. Une destination CUE bloquée par un dossier provoque le retour arrière de la piste déjà remplacée : sa copie WAVE précédente est restaurée, le dossier reste intact et la source est inchangée. Le banc, tous ses WAV/CUE et les exécutables temporaires sont supprimés après contrôle.

Audit : 1 398 fichiers C++, 56 clés App, 29 langues ; constantes conformes. Aucune traduction nouvelle n’a été nécessaire, les diagnostics CUE/WAVE et d’écriture existants sont réutilisés.

La façade runtime complète, les disques runtime modifiables, les autres opérations de préparation et leur raccordement au consommateur restent dans le portage ; ce bloc ne les déclare pas achevés.
