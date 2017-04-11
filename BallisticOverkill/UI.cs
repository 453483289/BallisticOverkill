using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UnityEngine;

namespace BallisticOverkill
{
    class UI
    {
        public const int mainWindow = 0;
        public const int visualsWindow = 1;
        public const int aimbotWindow = 2;
        public const int removalsWindow = 3;
        public const int miscWindow = 4;
        public Rect MainWindowRect = new Rect(5, 5, 115, 150);
        public Rect AimbotWindowRect = new Rect(100, 5, 175, 150);
        public Rect VisualsWindowRect = new Rect(200, 5, 175, 150);
        public Rect RemovalsWindowRect = new Rect(100, 150, 175, 150);
        public Rect MiscWindowRect = new Rect(200, 150, 200, 250);
        public bool VisualsWindow = false;
        public bool AimbotWindow = false;
        public bool RemovalsWindow = false;
        public bool MiscWindow = false;

        public UI()
        {

        }

        public void RenderUI()
        {
            MainWindowRect = GUI.Window(mainWindow, MainWindowRect, RenderWindows, "BO Cheetos");

            if(VisualsWindow)
                VisualsWindowRect = GUI.Window(visualsWindow, VisualsWindowRect, RenderWindows, "[Visuals]");
            if(AimbotWindow)
                AimbotWindowRect = GUI.Window(aimbotWindow, AimbotWindowRect, RenderWindows, "[Aimbot]");
            if(RemovalsWindow)
                RemovalsWindowRect = GUI.Window(removalsWindow, RemovalsWindowRect, RenderWindows, "[Removals]");
            if(MiscWindow)
                MiscWindowRect = GUI.Window(miscWindow, MiscWindowRect, RenderWindows, "[Misc]");
        }

        public void RenderWindows(int windowId)
        {
            switch(windowId) {
                case mainWindow:
                    GUI.BeginGroup(new Rect(2, 17, MainWindowRect.width - 4, MainWindowRect.height - 15));
                    if(GUI.Button(new Rect(5, 5, 100, 25), "Visuals"))
                        VisualsWindow = !VisualsWindow;
                    if(GUI.Button(new Rect(5, 35, 100, 25), "Aimbot"))
                        AimbotWindow = !AimbotWindow;
                    if(GUI.Button(new Rect(5, 65, 100, 25), "Removals"))
                        RemovalsWindow = !RemovalsWindow;
                    if(GUI.Button(new Rect(5, 95, 100, 25), "Misc"))
                        MiscWindow = !MiscWindow;
                    GUI.EndGroup();
                    break;
                case visualsWindow:
                    GUI.BeginGroup(new Rect(2, 17, VisualsWindowRect.width - 4, VisualsWindowRect.height - 15));
                    Options.ESPBoxes = GUI.Toggle(new Rect(5, 5, 100, 25), Options.ESPBoxes, "2D Boxes");
                    Options.Snaplines = GUI.Toggle(new Rect(5, 30, 100, 25), Options.Snaplines, "Snaplines");
                    GUI.EndGroup();
                    break;
                case aimbotWindow:
                    GUI.BeginGroup(new Rect(2, 17, AimbotWindowRect.width - 4, AimbotWindowRect.height - 15));
                    GUI.Label(new Rect(5, 5, 150, 60), "Not working yet. GO AWAY!!");
                    GUI.EndGroup();
                    break;
                case removalsWindow:
                    GUI.BeginGroup(new Rect(2, 17, RemovalsWindowRect.width - 4, RemovalsWindowRect.height - 15));
                    Options.NoRecoil = GUI.Toggle(new Rect(5, 5, 100, 25), Options.NoRecoil, "No Recoil");
                    Options.NoSpread = GUI.Toggle(new Rect(5, 30, 150, 25), Options.NoSpread, "No Spread");
                    GUI.EndGroup();
                    break;
                case miscWindow:
                    GUI.BeginGroup(new Rect(2, 17, MiscWindowRect.width - 4, MiscWindowRect.height - 15));
                    Options.EnableDmgMod = GUI.Toggle(new Rect(5, 5, 190, 25), Options.EnableDmgMod, $"Damage per shot ({(int)Options.DamagePerShot}):");
                    Options.DamagePerShot = GUI.HorizontalSlider(new Rect(5, 25, 190, 25), Options.DamagePerShot, 10.0f, 1000.0f);
                    Options.EnableRoFMod = GUI.Toggle(new Rect(5, 40, 190, 25), Options.EnableRoFMod, $"Rate of Fire ({(int)Options.RateOfFire}):");
                    Options.RateOfFire = GUI.HorizontalSlider(new Rect(5, 60, 190, 25), Options.RateOfFire, 10.0f, 50.0f);
                    Options.InstantBoltAction = GUI.Toggle(new Rect(5, 80, 190, 25), Options.InstantBoltAction, "Instant Bolt Action");
                    Options.InstantReload = GUI.Toggle(new Rect(5, 100, 190, 25), Options.InstantReload, "Instant Reload");
                    Options.FullAutoWeapons = GUI.Toggle(new Rect(5, 120, 190, 25), Options.FullAutoWeapons, "FullAuto Weapons");
                    Options.InfiniteAmmo = GUI.Toggle(new Rect(5, 140, 190, 25), Options.InfiniteAmmo, "Infinite Ammo");
                    Options.EnableSpeedMod = GUI.Toggle(new Rect(5, 155, 190, 25), Options.EnableSpeedMod, $"Speed Multiplier ({(int)Options.SpeedMultiplier}):");
                    Options.SpeedMultiplier = GUI.HorizontalSlider(new Rect(5, 175, 190, 25), Options.SpeedMultiplier, 1.0f, 25.0f);
                    Options.EnableScaleMod = GUI.Toggle(new Rect(5, 195, 190, 25), Options.EnableScaleMod, $"Model Scale ({(int)Options.ModelScale}):");
                    Options.ModelScale = GUI.HorizontalSlider(new Rect(5, 215, 190, 25), Options.ModelScale, 1.0f, 10.0f);
                    GUI.EndGroup();
                    break;
                default:
                    break;
            }
            GUI.DragWindow();
        }
    }
}
