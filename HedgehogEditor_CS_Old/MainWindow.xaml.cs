using System;

using System.Windows;

using SFML.Graphics;
using SFML.System;
using SFML.Window;
using System.Windows.Threading;


namespace HedgehogEditor
{

    public partial class MainWindow : System.Windows.Window
    {

        private Vector2f camera_pos;

        private float cam_scale = 2.0f;

        DateTime prevTime;
        private Tile currentTile = new Tile(new Texture("CPZ.png"));
        private Room currentRoom;
        private RenderWindow _renderWindow;
        private RenderTexture _renderTexture;
        //private readonly CircleShape _circle;
        private readonly DispatcherTimer _timer;
        public enum Tool
        {
            Brush,
            Pencil,
            Fill,
        }
        public Tool tool = Tool.Pencil;
        public MainWindow()
        {
            this.InitializeComponent();

            currentRoom = new Room();

            
            this.CreateRenderWindow();
            
            var refreshRate = new TimeSpan(0, 0, 0, 0, 1000 / 120);
            this._timer = new DispatcherTimer { Interval = refreshRate };
            this._timer.Tick += Timer_Tick;
            this._timer.Start();
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
        }

        private void CreateRenderWindow()
        {
            if (this._renderWindow != null)
            {
                this._renderWindow.SetActive(false);
                this._renderWindow.Dispose();
            }
            

            var context = new ContextSettings { DepthBits = 24 };
            this._renderWindow = new RenderWindow(this.DrawSurface.Handle, context);
            this._renderWindow.MouseButtonPressed += RenderWindow_MouseButtonPressed;
            this._renderWindow.KeyPressed += RenderWindow_KeyPressed;

            this._renderWindow.SetActive(true);
            
            this._renderTexture = new RenderTexture(_renderWindow.Size.X / 2, _renderWindow.Size.Y / 2);
            
            
        }

        private void DrawSurface_SizeChanged(object sender, EventArgs e)
        {
            this.CreateRenderWindow();
            //this._renderWindow.Size = new Vector2u((uint)DrawSurface.Size.Width, (uint)DrawSurface.Size.Height);
        }

        private void RenderWindow_KeyPressed(object sender, SFML.Window.KeyEventArgs e)
        {
            switch(e.Code)
            {
                case Keyboard.Key.Left:
                    currentTile.textureIndex.X--;
                    break;

                case Keyboard.Key.Right:
                    currentTile.textureIndex.X++;
                    break;

                case Keyboard.Key.Up:
                    currentTile.textureIndex.Y--;
                    break;

                case Keyboard.Key.Down:
                    currentTile.textureIndex.Y++;
                    break;
            }
        }
        private void RenderWindow_MouseButtonPressed(object sender, SFML.Window.MouseButtonEventArgs e)
        {
            if (tool == Tool.Pencil)
            {
                Vector2i index = getCursorIndex(16, new Vector2i(e.X, e.Y));
                if (index.X < 0 ||
                    index.Y < 0 ||
                    index.X > 64 * 16 ||
                    index.Y > 8 * 16)
                    return;
                currentRoom.map[0, index.X, index.Y] = new Tile(currentTile);
            }
        }
        private Vector2i getCursorIndex(int gridSize, Vector2i mousePos)
        {
            return 
                (mousePos + new Vector2i(
                    (int)camera_pos.X * (int)cam_scale, 
                    (int)camera_pos.Y * (int)cam_scale)
                    ) 
                    / (int)(gridSize * cam_scale);
        }
        private void Timer_Tick(object sender, EventArgs e)
        {
            TimeSpan dt = DateTime.Now - prevTime;
            
            this._renderWindow.DispatchEvents();


            int scrollspd = 1;

            if (Keyboard.IsKeyPressed(Keyboard.Key.A))
                camera_pos.X -= scrollspd / cam_scale * dt.Milliseconds;
            if (Keyboard.IsKeyPressed(Keyboard.Key.D))
                camera_pos.X += scrollspd / cam_scale * dt.Milliseconds;
            if (Keyboard.IsKeyPressed(Keyboard.Key.W))
                camera_pos.Y -= scrollspd / cam_scale * dt.Milliseconds;
            if (Keyboard.IsKeyPressed(Keyboard.Key.S))
                camera_pos.Y += scrollspd / cam_scale * dt.Milliseconds;





            this._renderWindow.Clear(SFML.Graphics.Color.Black);
            this._renderTexture.Clear(SFML.Graphics.Color.Black);
            for (int l = 0; l < currentRoom.map.GetLength(0); l++) {
                for (int x = (int)camera_pos.X / 16;
                    x < ((int)camera_pos.X + _renderTexture.Size.X) / 16 + 1;
                    x++)
                {
                    for (int y = (int)camera_pos.Y / 16;
                        y < ((int)camera_pos.Y + _renderTexture.Size.Y) / 16 + 1;
                        y++)
                    {
                        if (x < 0 ||
                            y < 0 ||
                            x >= 64 * 16 ||
                            y >= 8 * 16)
                            continue;
                        if (currentRoom.map[l, x, y] != null)
                        {
                            currentRoom.map[l, x, y].Draw(_renderTexture, RenderStates.Default, new Vector2i(x, y));
                        }
                    }
                }
            }
            CircleShape _circle = new CircleShape(20) { FillColor = SFML.Graphics.Color.Magenta };

            Vector2f cam_size = new Vector2f(_renderTexture.Size.X, _renderTexture.Size.Y);
            this._renderTexture.SetView(new View(camera_pos + cam_size / 2 , cam_size));
            this._renderTexture.Display();
            Sprite spr = new Sprite(_renderTexture.Texture);
            spr.Color = SFML.Graphics.Color.White;

            spr.Scale = new Vector2f(cam_scale, cam_scale);
            this._renderWindow.Draw(spr);

            //this._renderWindow.Draw(_circle);
            this._renderWindow.Display();
            prevTime = DateTime.Now;
        }



        private void ExitCommand_CanExecute(object sender, System.Windows.Input.CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = true;
        }

        private void ExitCommand_Executed(object sender, System.Windows.Input.ExecutedRoutedEventArgs e)
        {
            Application.Current.Shutdown();
        }
        private void NewTilemapCommand_CanExecute(object sender, System.Windows.Input.CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = true;
        }

        private void NewTilemapCommand_Executed(object sender, System.Windows.Input.ExecutedRoutedEventArgs e)
        {
            //New Tilemap
        }
        private void OpenRoomCommand_CanExecute(object sender, System.Windows.Input.CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = true;
        }

        private void OpenRoomCommand_Executed(object sender, System.Windows.Input.ExecutedRoutedEventArgs e)
        {
            currentRoom.Load("room.room");
        }
    }
    
}
