namespace Weather_Station
{
    public partial class App : Application
    {
        public static bool isConfig = false;
        public static bool hasAddress = true;
        public App()
        {
            InitializeComponent();

            MainPage = new AppShell();
        }
    }
}
