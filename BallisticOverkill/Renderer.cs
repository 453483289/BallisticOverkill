using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UnityEngine;
using UnityEngine.UI;

namespace BallisticOverkill
{
    public class Renderer
    {
        public static Material mat;

        public static void Begin()
        {
            if(!mat) {
                // Unity has a built-in shader that is useful for drawing
                // simple colored things. In this case, we just want to use
                // a blend mode that inverts destination colors.			
                var shader = Shader.Find("Hidden/Internal-Colored");
                mat = new Material(shader);
                mat.hideFlags = HideFlags.HideAndDontSave;
                // Set blend mode to invert destination colors.
                mat.SetInt("_SrcBlend", (int)UnityEngine.Rendering.BlendMode.SrcColor);
                mat.SetInt("_DstBlend", (int)UnityEngine.Rendering.BlendMode.SrcColor);
                // Turn off backface culling, depth writes, depth test.
                mat.SetInt("_Cull", (int)UnityEngine.Rendering.CullMode.Off);
                mat.SetInt("_ZWrite", 0);
                mat.SetInt("_ZTest", (int)UnityEngine.Rendering.CompareFunction.Always);
            }
            
            GL.PushMatrix();
            mat.SetPass(0);
            GL.LoadOrtho();
        }
        public static void End()
        {
            GL.PopMatrix();
        }

        public static void RenderLine(Vector2 p1, Vector2 p2, Color clr)
        {
            GL.Begin(GL.LINES);
            {
                GL.Color(clr);
                GL.Vertex3(p1.x, p1.y, 0);
                GL.Vertex3(p2.x, p2.y, 0);
            }
            GL.End();
        }
        public static void RenderFilledQuad(Rect rect, Color clr)
        {
            GL.Begin(GL.QUADS);
            {
                GL.Color(clr);
                GL.Vertex3(rect.xMin, rect.yMin, 0);
                GL.Vertex3(rect.xMax, rect.yMin, 0);
                GL.Vertex3(rect.xMax, rect.yMax, 0);
                GL.Vertex3(rect.xMin, rect.yMax, 0);
            }
            GL.End();
        }
        public static void RenderQuad(Rect rect, Color clr)
        {
            GL.Begin(GL.LINES);
            {
                GL.Color(clr);
                GL.Vertex3(rect.xMin, rect.yMin, 0);
                GL.Vertex3(rect.xMax, rect.yMin, 0);

                GL.Vertex3(rect.xMax, rect.yMin, 0);
                GL.Vertex3(rect.xMax, rect.yMax, 0);

                GL.Vertex3(rect.xMax, rect.yMax, 0);
                GL.Vertex3(rect.xMin, rect.yMax, 0);

                GL.Vertex3(rect.xMin, rect.yMax, 0);
                GL.Vertex3(rect.xMin, rect.yMin, 0);
            }
            GL.End();
        }
    }
}
