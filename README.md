# GBS-scrollScenePlugin
 Single Scene Screen Scroll plugin for GBS

How to use:
unzip and install in the plugin folder, if you are using Platformer+ or some other plugin, some conflicts might occur.
all default scene types are supported. Each scene must preferably be screen size (20 8pxtiles wide and 18 8pxtile tall)
you can also make it smaller (for example 16 tile tall) if you are displaying a 2 tile tall hud at the bottom of the screen.

To be able to make a scene able to screen scroll into another scene, add the "add neighbour scene" event in the init of the scene.
There is no need to add a trigger on the edge of the scene with a "change scene event" on it. The plugin takes care of it.
Click the rounding tile checkbox if you are using the TopDown scene type.

Also make sure you use a common tileset between the scene that you are scrolling to. (Click on the puzzle piece on a scene to assign a common tileset)


https://github.com/Mico27/GBS-scrollScenePlugin/assets/32064874/bf08c9e1-6e72-4e7b-8fd9-ec2462bc76a9

