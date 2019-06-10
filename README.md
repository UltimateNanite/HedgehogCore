# HedgehogCreator

HedgehogCreator is an SFML-based toolkit for creating Sonic fangames, meant to accurately replicate the gameplay and physics of the 90s-era games.
The kit consists of two programs: **HedgehogEditor**, the level editor, and **HedgehogCore**, the runtime.


NOTE: HedgehogCreator is not yet in a usable state, and this repository currently exists solely for version control purposes. However, if you're interested, you're more than welcome to try out what works so far.

The features currently implemented include:
- (Near-complete) Genesis Sonic gameplay
- Accurate tile-based collision
- Parallax backgrounds
- Game objects with Unity-esque component-based behaviour
  ( Implemented components include: )
    - SpriteRenderer
    - SheetAnimator
    - CollisionHazard

Features currently in development

- The accompanying editor HedgehogEditor
- Saving & loading of rooms

Planned features include
- Rings
- UI elements
- Tails & Knuckles' gameplay
- A more sophisticated and extensible collision system
- Tilemap editing

And yeah, the codebase is a bit of a mess.
The engine is a personal project. It's not meant to be professional, and the code quality often reflects that.
It also doesn't help that I wrote a big part of it when I was still learning C++.

Alright, that's about it.
