using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using SFML.Graphics;
using SFML.System;
namespace HedgehogEditor
{
    class Tile
    {
        public string texturename;
        public Texture texture;
        public byte[] downHeightMap   = new byte[16];
        public byte[] rightHeightMap  = new byte[16];
        public byte[] upHeightMap     = new byte[16];
        public byte[] leftHeightMap   = new byte[16];
        public Vector2i textureIndex;
        
        public Tile(Texture texture)
        {
            this.texture = texture;
            for (int i = 0; i < 16;i++)
            {
                downHeightMap[i]    = 15;
                rightHeightMap[i]   = 15;
                upHeightMap[i]      = 15;
                leftHeightMap[i]    = 15;
            }
        }
        public Tile(Tile tile)
        {
            this.texture = tile.texture;
            this.downHeightMap = tile.downHeightMap;
            this.rightHeightMap = tile.rightHeightMap;
            this.upHeightMap = tile.upHeightMap;
            this.leftHeightMap = tile.leftHeightMap;
        }
        public void Draw(RenderTarget target, RenderStates states, Vector2i index)
        {
            Sprite spr = new Sprite();
            spr.Texture = texture;
            //RectangleShape spr = new RectangleShape(new Vector2f(16, 16));
            spr.TextureRect = new IntRect(textureIndex * 16, new Vector2i(16, 16));
            spr.Position = new Vector2f(index.X * 16, index.Y * 16);
            spr.Color = new Color(255, 255, 255, 255);
            target.Draw(spr, states);
        }
        static public byte[] GenerateHeightArray(Image arrayImg, Vector2i pos, int groundMode)
        {


            byte[] temp = new byte[16];
            if (groundMode == 0 || groundMode == 2)
            {
                for (byte x = 0; x < 16; x++)
                {
                    bool pixelFound = false;

                    if (groundMode == 0)
                    {
                        for (byte y = 0; y < 16 && !pixelFound; y++)
                        {
                            if (arrayImg.GetPixel(
                                (uint)(pos.X * 16 + x), 
                                (uint)(pos.Y * 16 + y)
                                ).A >= 0.5f)
                            {
                                temp[x] = (byte)(16 - y);
                                pixelFound = true;
                            }
                        }
                    }
                    else
                    {
                        for (byte y = 15; y > 0 && !pixelFound; y--)
                        {
                            Color temp2 = arrayImg.GetPixel(
                                (uint)(pos.X * 16 + x),
                                (uint)(pos.Y * 16 + y)
                                );
                            if (temp2.A >= 0.5f)
                            {
                                temp[x] = (byte)(y + 1);
                                pixelFound = true;
                            }
                        }
                    }

                }

            }
            else if (groundMode == 3 || groundMode == 1)
            {
                if (groundMode == 3)
                {
                    for (byte y = 0; y < 16; y++)
                    {
                        bool pixelFound = false;
                        for (byte x = 0; x < 16 && !pixelFound; x++)
                        {
                            if (arrayImg.GetPixel(
                                (uint)(pos.X * 16 + x),
                                (uint)(pos.Y * 16 + y)
                                ).A >= 0.5f)
                            {
                                temp[y] = (byte)(16 - x);
                                pixelFound = true;
                            }
                        }
                    }
                }
                else
                {
                    for (byte y = 0; y < 16; y++)
                    {
                        bool pixelFound = false;
                        for (byte x = 15; x > 0 && !pixelFound; x--)
                        {
                            if (arrayImg.GetPixel(
                                (uint)(pos.X * 16 + x),
                                (uint)(pos.Y * 16 + y)
                                ).A >= 0.5f)
                            {
                                temp[y] = (byte)(x + 1);
                                pixelFound = true;
                            }
                        }
                    }
                }
            }
            return temp;
        }
    }
}
