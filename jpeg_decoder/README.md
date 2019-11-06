Décodeur JPEG to PPM/PGM
==                  
* HACHIN Vincent
* IOLLO Jacopo          
* OUDOUMANESSAH Geoffroy


### Description générale du module jpeg2ppm:
<p>
Vous trouverez ici un module permettant de convertir des images en SOF0 (baseline jpeg).

<strong>Utilisation du logiciel</strong>: jpeg2ppm <em>path/image_encodee.jpeg</em>
L'image décodée est placée dans le même fichier que l'image source.

Le objets en <em>.o</em> se trouvent dans <em>./obj</em>, l'exécutable dans <em>./bin</em>, le code source dans <em>./src</em>, et les éditions de liens dans <em>./include</em>.

Le code est commenté, la majorité des fonctions sont documentées. Nous avons volontairement laissé nos fonctions de test de chaque étape du décodage, soit le débugage se trouve dans les fonction <strong>test_*</strong> soit dans les fonctions du code source sous forme de commentaire.

</p>


### Commandes du Makefile:
<p>
<strong>make</strong> : génération de l'executable <em>jpeg2ppm</em>.

<strong>make test</strong> : compilation et affichage de toutes les images de l'ensemble de test, les <em>.o</em> et l'exécutable <em>jpeg2ppm</em>.

<strong>make example</strong> : décodage de invader.jpeg.

<strong>make photo.jp(e)g</strong> : décodage de <em>images/photo.jp(e)g</em>.

<strong>make clean</strong> : Supprime les images décodées en <em>.ppm</em> ou en <em>.pgm</em>, les fichiers objets et l'executable.
</p>
