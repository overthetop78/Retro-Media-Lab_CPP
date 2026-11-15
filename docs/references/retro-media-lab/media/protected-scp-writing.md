# Conversion de pistes protégées vers SCP

Source C# : Images/Formats/Floppy/Scp/ProtectedTrackScpImageAdapter.cs, avec ScpMetadataKeys, ScpFlags, ScpWriterDefaults et MediaFileSystems/Functions/MediaMetadataFunctions.cs.

Scp::fromProtected construit le modèle SCP depuis les révolutions communes sans modifier l'entrée. Le nombre écrit est le minimum des nombres de révolutions des pistes. Les révolutions supplémentaires servent néanmoins à déterminer si la résolution est commune, comme en C#.

Une résolution valide stockée dans les métadonnées est prioritaire. Sinon, une résolution commune multiple de 25 ns est conservée si son indice tient dans un octet ; à défaut, 25 ns. Les intervalles et la durée d'index sont convertis par arithmétique entière avec arrondi et minimum d'un tick ; un dépassement uint32 est rejeté.

Les métadonnées décimales acceptent uniquement les chiffres, sans signe ni espaces, conformément à NumberStyles.None du C#. Version, type, drapeaux et largeur sont repris s'ils sont valides. Footer et Extended sont retirés, les faces sont recalculées depuis les pistes, triées par numéro SCP. Les pistes sans révolutions, les nombres incompatibles et les faces hors 0/1 sont refusés.

L'API reçoit les métadonnées explicitement et réutilise les diagnostics traduits existants, ainsi que le writer atomique SCP. L'annulation est vérifiée pendant le parcours, y compris pour chaque intervalle. Les modèles et buffers sont possédés par RAII.

Ce travail porte le convertisseur complet. Le catalogue bitcell et le raccordement à la composition globale des writers restent à intégrer.
