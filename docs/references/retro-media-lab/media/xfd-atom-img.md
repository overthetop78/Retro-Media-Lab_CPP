# XFD, Atom DSK et IMG général

Références C# en lecture seule : `Xfd/XfdReader.cs`, `AcornAtom/AcornAtomDskReader.cs`, `Raw/RawImgReader.cs` et le paramètre de numérotation par défaut de `LinearSectorImageGeometry`.

XFD reprend quatre profils Atari 90/130/140/180. La sonde accepte la longueur du profil ou celle augmentée de 16 octets. La lecture exige que ce padding supplémentaire soit nul. Le profil double densité garde les trois premiers secteurs de 128 octets puis les secteurs de 256 octets ; le padding n’augmente pas la capacité logique. La géométrie Atari partagée est réutilisée, avec l’identifiant de conteneur XFD approprié.

Atom DSK reprend 40 cylindres, une tête, dix secteurs de 256 octets et une numérotation à partir de zéro. Il valide le nombre d’entrées et le nombre de secteurs du catalogue avec le décodeur de champs DFS public. Comme en C#, un catalogue avec total nul et au moins une entrée peut être reconnu. Les extensions acceptées sont DSK, IMA et IMG.

Le reader IMG général construit d’abord une image IBM. En l’absence de BPB FAT, il demande à MFS la sonde de répertoire système CPC puis la spécification PCW. Il réidentifie l’image sans recopier ses secteurs. Un BPB FAT conservant le profil IBM a priorité sur ces deux réidentifications. Engine ne dépend pas directement d’Analysis.

Les trois readers utilisent les contrats communs, les diagnostics traduits et la politique de rejet de lecture. Les erreurs E/S et d’annulation ne sont pas absorbées. La composition globale et l’intégration App ne sont pas achevées par ces readers.
