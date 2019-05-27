
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using SFML.System;
namespace HedgehogEditor
{
    struct TextureRefInfo
    {
        public string name;
        public Vector2i textureRectIndex;

        public TextureRefInfo(string name, int indexX, int indexY) : this()
        {
            this.name = name;
            this.textureRectIndex = new Vector2i(indexX, indexY);
        }
    }
    class Room
    {
        private const int TILEMAPID = -2;
        public Tile[,,] map = new Tile[2, 64 * 16, 8 * 16];



        public Dictionary<string, SFML.Graphics.Texture> tilemaptextures;
        public Dictionary<string, Tile[,]> tilemapdata;
        public void Save(string path, string assetspath) {
            if (File.Exists(path))
            {
                File.Delete(path);
            }

            //Create the file.
            using (FileStream fs = File.Create(path))
            {

                foreach (var s in tilemaptextures) {
                    AddText(fs, TILEMAPID + " " + s.Key + "\n");
                }
                for (int l = 0; l < map.GetLength(0); l++) {
                    for (int x = 0; x < 16 * 64; x++) {
                        for (int y = 0; y < 16 * 8; y++) {
                            int cx = x / 16;
                            int cy = y / 16;
                            TextureRefInfo currentTileRef = TileToTextureRefInfo(map[0,x,y]);
                            if (currentTileRef.name != "") {
                                AddText(fs, l + ":" + cx + ":" + cy + ":" + x % 16 + ":" + y % 16 + ":" + 0 + currentTileRef.name + '|' + "," + currentTileRef.textureRectIndex.X + "," + currentTileRef.textureRectIndex.Y + ".");
                            }
                        }
                    }
                }
                AddText(fs, "-1"); //End of Tiles

                foreach (var it in tilemapdata) {
                    string filepath = assetspath + "/" + it.Key + ".tdata";
                    if (File.Exists(filepath))
                    {
                        File.Delete(filepath);
                    }
                    FileStream fileout = File.Create(filepath);
                    AddText(fs, TilemapToString(it.Value));
                }
            }
        }


        private int GetStreamReaderNumber(StreamReader reader)
        {
            
            char first = (char)reader.Read();
            if ((first < '0' || first > '9') && first != '-')
                throw new FormatException();
            else
            {
                string result = Char.ToString(first);

                if (reader.Peek() >= '0' && reader.Peek() <= '9')
                {

                    char input = (char)reader.Read();
                    while (input >= '0' && input <= '9')
                    {
                        result += input;
                        if (reader.Peek() < '0' || reader.Peek() > '9')
                            break;

                        input = (char)reader.Read();
                    }
                }
                int intResult = 0;
                Int32.TryParse(result, out intResult);
                return intResult;
            }
        }
        public void Load(string path) {
            if (!File.Exists(path))
            {
                throw new FileNotFoundException();
            }
            FileStream ifs = File.OpenRead(path);
            Console.WriteLine("Loading room \"" + path+ "\"");
            StreamReader reader = new StreamReader(ifs);
	        while (!reader.EndOfStream) {
		
		        //Tile refs
		        int l = 0;
		        int cx = 0;
		        int cy = 0;
		        int x = 0;
		        int y = 0;
		        char del1 = '\0', del2 = '\0', del3 = '\0', del4 = '\0', del5 = '\0', del6 = '\0', del7 = '\0', del8 = '\0';
		        TextureRefInfo currentTileRef = new TextureRefInfo("",0,0);

                
                l = GetStreamReaderNumber(reader);
                if (l < 0)
                {

                    if (l == -1)
                    {
                        del1 = (char)reader.Read();
                        continue;
                    }
                    if (l == TILEMAPID)
                    {
                        reader.Read(); //Skip space


                        string tilemapname = "";
                        while (reader.Peek() != '\n' && reader.Peek() != '\r')
                            tilemapname += (char)reader.Read();
                        //ifs >> del1;
                        AddTileMap(tilemapname);


                        //Clever while loop thing, doesn't need a body
                        while ((char)reader.Read() == '\n' || (char)reader.Read() == '\r') ;

                        continue;
                    }
                }
                
		        int delim = 0;
                //ifs >> del1 >> cx >> del2 >> cy >> del3 >> x >> del4 >> y >> del5 >> delim;
                
                del1    = (char)reader.Read();
                cx      = GetStreamReaderNumber(reader);
                del2    = (char)reader.Read();
                cy      = GetStreamReaderNumber(reader);
                del3    = (char)reader.Read();
                x       = GetStreamReaderNumber(reader);
                del4    = (char)reader.Read();
                y       = GetStreamReaderNumber(reader);
                del5    = (char)reader.Read();
                delim   = GetStreamReaderNumber(reader);

                char namein = '\0';
		        while (true) {
			        namein = (char)reader.Read();
			        if (namein == '|')
				        break;
			        currentTileRef.name += namein;
		        }

                //ifs >> del6 >> currentTileRef.textureRectIndex.x >> del7 >> currentTileRef.textureRectIndex.y >> del8;
                del6 = (char)reader.Read();
                currentTileRef.textureRectIndex.X = GetStreamReaderNumber(reader);
                del7 = (char)reader.Read();
                currentTileRef.textureRectIndex.Y = GetStreamReaderNumber(reader);
                del8 = (char)reader.Read();
                map[l, cx * 16 + x, cy * 16 + y] = TextureRefToTile(currentTileRef);
		        if (x == -1 || reader.EndOfStream) break;

			


	        }
        }
        Tile[,] LoadTileData(string tilemapname, SFML.Graphics.Texture texture) {

            if (!File.Exists(tilemapname))
                throw new FileNotFoundException();


            Tile[,] result = new Tile[texture.Size.X / 16, texture.Size.Y / 16];
		    int x = 0;
            int y = 0;
            char del1 = '\0', del2 = '\0', del3 = '\0', del4 = '\0';
            
		    
            FileStream stream = File.OpenRead(tilemapname);
            StreamReader reader = new StreamReader(stream);

            while (!reader.EndOfStream) {
                Tile tile = new Tile(texture);
                //ifs >> x >> del1 >> y >> del2 >> del3 >> del4;
                x = reader.Read();
                del1 = (char)reader.Read();
                y = reader.Read();
                del2 = (char)reader.Read();
                del3 = (char)reader.Read();
                del4 = (char)reader.Read();

                if (del1 != ',')
			        break;

		        for (int it = 0; it< 4; it++) {
			        byte i = 0;
                    List<byte> hData = new List<byte>();
                    
			        while (del4 != '}') {
                        i = (byte)reader.Read();
                        del4 = (char)reader.Read();

				        hData.Add(i);
				        if (del4 != ',' && del4 != '}')
					        break;
			        }
                    del4 = (char)reader.Read();

                    tile.textureIndex = new Vector2i(x, y);
			
			        if (it == 0)
                        tile.downHeightMap = hData.ToArray();
			        if (it == 3)
                        tile.rightHeightMap = hData.ToArray();
			        if (it == 2)
                        tile.upHeightMap = hData.ToArray();
			        if (it == 1)
                        tile.leftHeightMap = hData.ToArray();
		        }

                result[x,y] = tile;
            }
	        return result;
        }
        private void AddTileMap(string tilemapname)
        {
            if (tilemaptextures == null)
                tilemaptextures = new Dictionary<string, SFML.Graphics.Texture>();
            if (tilemapdata == null)
                tilemapdata = new Dictionary<string, Tile[,]>();


            SFML.Graphics.Texture txt;
            //remove projects from design
            txt = new SFML.Graphics.Texture(tilemapname);
            tilemaptextures.Add(tilemapname, txt);

            Tile[,] result = new Tile[txt.Size.X / 16, txt.Size.Y / 16];

            SFML.Graphics.Image img = txt.CopyToImage();

            if (File.Exists(tilemapname + ".tdata"))
            {

            }
            else
            {
                for (int x = 0; x < txt.Size.X / 16; x++)
                {

                    for (int y = 0; y < txt.Size.Y / 16; y++)
                    {
                        if (result[x, y] == null)
                            result[x, y] = new Tile(txt);

                        Tile s = result[x, y];
                        Vector2i txtIndex = new Vector2i(x, y);
                        s.texturename = tilemapname;
                        s.textureIndex = txtIndex;
                        s.downHeightMap = Tile.GenerateHeightArray(img, txtIndex, 0);
                        s.rightHeightMap = Tile.GenerateHeightArray(img, txtIndex, 3);
                        s.upHeightMap = Tile.GenerateHeightArray(img, txtIndex, 2);
                        s.leftHeightMap = Tile.GenerateHeightArray(img, txtIndex, 1);
                    }
                }
            }

            tilemapdata.Add(tilemapname, result);
        }

        private TextureRefInfo TileToTextureRefInfo(Tile tile)
        {
            if (tile == null) return new TextureRefInfo("", -1, -1);
            return new TextureRefInfo(tile.texturename, tile.textureIndex.X, tile.textureIndex.Y);
        }
        private Tile TextureRefToTile(TextureRefInfo refInfo)
        {
            return tilemapdata[refInfo.name][refInfo.textureRectIndex.X, refInfo.textureRectIndex.Y];
        }

        private static void AddText(FileStream fs, string value)
        {
            byte[] info = new UTF8Encoding(true).GetBytes(value);
            fs.Write(info, 0, info.Length);
        }
        private static string HeightArrayToString(byte[] data)
        {
            string result = "{";
            foreach (var b in data)
            {
                result += b.ToString() + ",";
            }
            result.TrimEnd(',');
            result += "}";
            return result;
        }
        private static string TilemapToString(Tile[,] data)
        {
            string result = "";
            for (int x = 0; x < data.GetLength(0); x++)
            {
                for (int y = 0; y < data.GetLength(1); y++)
                {
                    Tile t = data[x, y];
                    result += x + "," + y + ":[";
                    result += HeightArrayToString(t.downHeightMap);
                    result += HeightArrayToString(t.rightHeightMap);
                    result += HeightArrayToString(t.upHeightMap);
                    result += HeightArrayToString(t.leftHeightMap);
                    result += "]";
                }
            }
            return result;
        }
    }
}