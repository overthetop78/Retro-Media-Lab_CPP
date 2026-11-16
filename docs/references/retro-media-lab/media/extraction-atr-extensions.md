# Extraction ATR et extensions — bloc 232

Référence : `Images/Conversion/Atari/AtrPayloadWriter.cs`, qui utilise `AtrReader.ReadValidatedContainerAsync` puis écrit tous les octets après les 16 octets de l’en-tête. La validation renvoie le conteneur original et ne remplace pas ses octets par l’image sectorielle reconstruite.

`extractAtrPayloadAsync` conserve ce comportement. Les octets de remplissage après les données déclarées restent présents dans la sortie, s’ils sont acceptés par le validateur ATR existant. Aucune normalisation, création de secteurs manquants ou conversion de géométrie n’intervient. La sortie atomique réutilise le mécanisme commun et conserve une destination préexistante si la validation échoue. Les chemins et l’annulation sont contrôlés avant l’écriture ; aucun nouveau texte d’erreur brut n’est ajouté.

La reconnaissance des sources et des fichiers associés utilise désormais `Primitives::pathExtensionUtf8`, fondé sur le même traitement de nom que les autres services natifs. Les noms `.IMG`, `file..IMG` et `file.` sont traités respectivement comme `.img`, `.img` et aucune extension pour le contexte normalisé. Les indices de fichiers associés restent comparés sans distinction de casse.

Compilation Debug réussie, 13 étapes annoncées dont uniquement les consommateurs des en-têtes modifiés, sans avertissement du compilateur. Le contrôle temporaire a vérifié la conservation des octets ATR et du remplissage externe, le rejet avant modification d’une destination existante, l’annulation, les chemins vides et la présélection par extension des fichiers associés. Il n’a retesté aucun décodeur sectoriel. Ses fichiers média ont été créés dans un dossier temporaire RAII interne au banc, puis supprimés, ainsi que le banc.

Le registre C# historique de politiques sectorielles n’est pas confondu avec le registre commun actuel : l’explorateur C# courant utilise directement `MediaImageReadingService`, comme l’explorateur C++. Cette observation évite de créer une seconde chaîne de reconnaissance en parallèle de celle déjà portée ; elle ne marque pas automatiquement comme portées les anciennes politiques encore signalées dans l’inventaire.
