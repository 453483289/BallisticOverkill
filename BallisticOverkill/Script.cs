using Aquiris.Ballistic.Game.Networking;
using Aquiris.Ballistic.Game.Networking.Events;
using Aquiris.Ballistic.Game.Services;
using Aquiris.Ballistic.Game.Utility;
using Aquiris.Ballistic.Network.Transport.Gameplay.Player.Requests;
using Aquiris.Services;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UnityEngine;
using UnityEngine.UI;

namespace BallisticOverkill
{
    public class Script : MonoBehaviour
    {
        private static GameObject self;
        private static UI ui;
        private bool ShowMenu = true;
        private static WeaponController lastWeapon;
        private bool killall = false;

        public static void Main()
        {
            self = new GameObject();
            self.AddComponent<Script>();
            DontDestroyOnLoad(self);

            ui = new UI();
        }

        private FPSCharacter FindBestTarget(LocalCharacter local)
        {
            return null;
        }

        private void RunAimbot(LocalCharacter local)
        {

        }

        public void Update()
        {
            if(Input.GetKeyDown(KeyCode.Insert))
                ShowMenu = !ShowMenu;

            if(Input.GetKeyDown(KeyCode.Delete))
                killall = true;

            var local = LocalCharacter.Instance;

            if(!local || local.isDead) return;
            
            if(local.currentWeapon.weaponId != lastWeapon?.weaponId) {
                if(lastWeapon != null) {
                    lastWeapon.aimController.aimData.ImportDataFromXml();
                    lastWeapon.shootController.shootData.ImportDataFromXml();
                    lastWeapon.animationData1st.ImportDataFromXml();
                }
                lastWeapon = local.currentWeapon;
            }
            
            if(Options.NoRecoil) {
                local.currentWeapon.aimController.aimData.aimingRecoilPercent = 0.0f;
                local.currentWeapon.aimController.aimData.crouchRecoilPercent = 0.0f;
                local.currentWeapon.aimController.aimData.recoilSpeed = 0.0f;
                local.currentWeapon.aimController.aimData.recoil = 0.0f;
                local.currentWeapon.aimController.aimData.upRecoil = 0.0f;
            }

            if(Options.NoSpread) {
                local.currentWeapon.aimController.aimData.precision = 100000.0f;
                local.currentWeapon.aimController.aimData.aimCrouchPrecision = 100000.0f;
                local.currentWeapon.aimController.aimData.aimPrecision = 100000.0f;
                local.currentWeapon.aimController.aimData.crouchPrecision = 100000.0f;
                local.currentWeapon.aimController.aimData.movePrecisionLossSpeed = 0.0f;
                local.currentWeapon.aimController.aimData.movePrecisionRecoverySpeed = 1000.0f;
                local.currentWeapon.aimController.aimData.shootPrecisionLossSpeed = 0.0f;
                local.currentWeapon.aimController.aimData.SprintAccuracyLoss = 0.0f;
            }

            SingletonBehaviour<GameSystem>.Instance.SpeedMultiplier = Options.SpeedMultiplier;

            if(Options.EnableDmgMod) {
                local.currentLoadout.HeroLevel = 17;
                local.GrenadeWeapon.shootController.shootData.magazineSize = 999;
                local.currentWeapon.shootController.shootData.damagePerShot = Options.DamagePerShot / 100.0f;
            }
                

            if(Options.EnableRoFMod)
                local.currentWeapon.shootController.shootData.fireRate = 1.0f / Options.RateOfFire;

            if(Options.InfiniteAmmo)
                local.currentWeapon.shootController.shootData.magazineSize = 99999;

            if(Options.InstantBoltAction) 
                local.currentWeapon.animationData1st.BoltActionDuration = 0.01f;
            if(Options.InstantReload)
                local.currentWeapon.animationData1st.ReloadDuration = 0.01f;
            if(Options.FullAutoWeapons)
                local.currentWeapon.aimController.shootController.shootData.shootMode = ShootMode.AUTOMATIC;
            
            if(killall) {
                killall = false;
                foreach(FPSCharacter fpsPlayer in NetworkCharacterList.Instance.GetAllConnectedCharacters()) {
                    if(!local.isDead && fpsPlayer.gameClient.spawned && !fpsPlayer.gameClient.isMe && fpsPlayer.IsEnemy) {
                        PlayerHitRequest evt = new PlayerHitRequest {
                            BodyPartHit = 0,
                            Damage = 9999.0,
                            ShooterPosition = new List<double>(3){ 0.0, 0.0, 0.0 },
                            UserBeingHitId = fpsPlayer.ownerID,
                            VictimPosition = new List<double>(3){ 0.0, 0.0, 0.0 },
                            WeaponId = -1,
                            DamageType = Aquiris.Ballistic.Network.Transport.Gameplay.Player.Events.EDamageType.HEADSHOT,
                            ShotsFired = 1,
                            Team = (sbyte)LocalCharacter.Instance.team
                        };
                        ServiceProvider.GetService<NetworkGameService>().RaiseNetworkEvent(evt);
                    }
                }
            }
        }

        public void OnGUI()
        {
            if(ShowMenu) 
                ui.RenderUI();

            var local = LocalCharacter.Instance;

            if(!local) return;
            
            Renderer.Begin();
            foreach(FPSCharacter fpsPlayer in NetworkCharacterList.Instance.GetAllConnectedCharacters()) {
                if(!local.isDead && fpsPlayer.gameClient.spawned && !fpsPlayer.gameClient.isMe) {
                    var screenHead = CameraUtils.SafeWorldToScreenPoint(local.StageCamera, fpsPlayer.HeadPivot.position);
                    var screenFeet = CameraUtils.SafeWorldToScreenPoint(local.StageCamera, fpsPlayer.transform.position);

                    float height = Math.Abs(screenFeet.y - screenHead.y);
                    float width = height * 0.6f;

                    var rect = new Rect((screenFeet.x - width / 2) / Screen.width, screenFeet.y / Screen.height, width / Screen.width, height / Screen.height);
                    
                    if(fpsPlayer.IsEnemy) {
                        if(Options.EnableScaleMod)
                            fpsPlayer.animatedComponent.transform.localScale = new Vector3(Options.ModelScale, Options.ModelScale, Options.ModelScale);
                        Color clr = local.IsCharacterVisible(fpsPlayer) ? Color.yellow : Color.red;
                        if(Options.ESPBoxes)
                            Renderer.RenderQuad(rect, clr);
                        if(Options.Snaplines) 
                            Renderer.RenderLine(new Vector2(0.5f, 0.0f), new Vector2((screenFeet.x) / Screen.width, screenFeet.y / Screen.height), clr);
                    } else {
                        if(Options.ESPBoxes)
                            Renderer.RenderQuad(rect, Color.blue);
                        if(Options.Snaplines)
                            Renderer.RenderLine(new Vector2(0.5f, 0.0f), new Vector2((screenFeet.x) / Screen.width, screenFeet.y / Screen.height), Color.blue);
                    }
                }
            }
            Renderer.End();
        }
    }
}
