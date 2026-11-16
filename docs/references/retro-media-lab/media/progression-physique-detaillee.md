# Progression physique détaillée

Référence C# : `Interfaces/IEtatLectureDisquette.cs` et `Interfaces/IPiste.cs`.

`PhysicalReadingState` porte l’étape, les comptes de pistes, les coordonnées facultatives, la tentative, la piste acquise, les états par piste, la clé de message, ses paramètres nommés et le message externe. La piste réutilise le contrat `DiskTrackInfo` existant. L’instantané est optionnel dans `PhysicalAcquisitionProgress` ; les appels précédents restent valables. Il n’y a aucun appel matériel ni dépendance GUI dans ce transport.

Compilation native réussie. Contrôle ciblé réussi : conservation de toutes les propriétés, durée de vie de la piste et compatibilité des appels sans détails. Contrôle temporaire supprimé après utilisation. Les fournisseurs matériels peuvent transmettre cet instantané par leur rappel de progression existant ; cette étape ne modifie pas leur exécution.
