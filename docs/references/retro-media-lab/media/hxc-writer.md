# Writer HxC et publication récupérable

Références C# en lecture seule : `Images/Formats/Floppy/HxcStream/HxcStreamWriter.cs` et `Images/Writing/AtomicMediaFileSetWriter.cs`, dans MediaEngine. Leurs empreintes correspondent à l'inventaire.

Le writer utilise les adaptateurs flux existants, complétés par une surcharge recevant le document. Il accepte les disquettes sans région faible, dans les limites physiques du format. Une capture native dont l'empreinte correspond est recopiée sans modification ; des données base64 invalides échouent sans recodage silencieux. Si l'empreinte change, le codec exact est utilisé. Le nom initial désigne la première piste physique et les sorties suivent l'ordre cylindre/tête.

`writeAtomicImageSet` prépare toutes les sorties avant publication. Les temporaires et sauvegardes utilisent des noms UUID dans le même répertoire. Les chemins normalisés sont uniques sans distinction de casse, comme en C#. La création NewOnly évite l'écrasement d'un temporaire existant. Le writer ne déplace jamais un répertoire utilisateur pour le traiter comme un ancien fichier.

La publication sauvegarde les anciennes sorties, puis publie les nouvelles. En cas d'exception ou d'annulation, la restauration parcourt tout le jeu en sens inverse. Contrairement au C#, elle poursuit ses tentatives après un premier échec ; les sauvegardes impossibles à restaurer sont conservées. Le nettoyage explicite signale ses erreurs et la destruction RAII effectue une dernière tentative sans exception. Cela ne constitue pas une transaction résistante aux coupures d'alimentation ni un verrouillage global contre des publications concurrentes.

La réservation positive est actuellement spécifique à Windows. L'adaptation hors Windows refuse explicitement cette demande ; la publication sans réservation reste indépendante de cette API. Aucune validation Linux/macOS n'est revendiquée. La création Qt NewOnly n'est pas présentée comme une reproduction du FileShare.None de .NET.

Sources primaires consultées le 16 novembre 2026 :

- [QFileDevice](https://doc.qt.io/qt-6/qfiledevice.html) : handle CRT, possession du fichier ouvert par nom, flush explicite nécessaire car close ignore ses erreurs.
- [FILE_ALLOCATION_INFO](https://learn.microsoft.com/en-us/windows/win32/api/winbase/ns-winbase-file_allocation_info) et [SetFileInformationByHandle](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-setfileinformationbyhandle) : réservation de stockage distincte de la longueur logique. Le handle CRT emprunté est converti par `_get_osfhandle` ; QFile reste responsable de sa fermeture.

L'intégration à la composition globale des services n'est pas achevée par cette tâche.
