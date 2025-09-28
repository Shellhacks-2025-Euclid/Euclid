using EuclidApp.Models;
using EuclidApp.Models.Shapes;
using EuclidApp.Utils;

namespace EuclidApp.Utils.Scaling
{
    public interface IShapeScaler
    {
        bool TryScale(SceneNode node, out Vec3f scale); 
    }


    public sealed class CubeScaler : IShapeScaler
    {
        public bool TryScale(SceneNode node, out Vec3f s)
        {
            s = default;
            if (node.Params is not CubeParamsVM p) return false;
            var edge = ScaleMath.ClampPos((float)p.Edge);
            s = new Vec3f(edge, edge, edge);
            return true;
        }
    }

    public sealed class SphereScaler : IShapeScaler
    {
        public bool TryScale(SceneNode node, out Vec3f s)
        {
            s = default;
            if (node.Params is not SphereParamsVM p) return false;
            var k = p.Radius > 0 ? ScaleMath.SafeRatio(p.Radius, 0.5) : 1f;
            s = new Vec3f(k, k, k);
            return true;
        }
    }

    public sealed class PlaneScaler : IShapeScaler
    {
        public bool TryScale(SceneNode node, out Vec3f s)
        {
            s = default;
            if (node.Params is not PlaneParamsVM p) return false;
            var sx = ScaleMath.ClampPos((float)p.Width);
            var sz = ScaleMath.ClampPos((float)p.Height);
            s = new Vec3f(sx, 1f, sz);
            return true;
        }
    }

    public sealed class ConeScaler : IShapeScaler
    {
        public bool TryScale(SceneNode node, out Vec3f s)
        {
            s = default;
            if (node.Params is not ConeParamsVM p) return false;
            var sxz = p.Radius > 0 ? ScaleMath.SafeRatio(p.Radius, 0.5) : 1f;
            var sy = p.Height > 0 ? ScaleMath.SafeRatio(p.Height, 1.0) : 1f;
            s = new Vec3f(sxz, sy, sxz);
            return true;
        }
    }

    public sealed class CylinderScaler : IShapeScaler
    {
        public bool TryScale(SceneNode node, out Vec3f s)
        {
            s = default;
            if (node.Params is not CylinderParamsVM p) return false;
            var sxz = p.Radius > 0 ? ScaleMath.SafeRatio(p.Radius, 0.5) : 1f;
            var sy = p.Height > 0 ? ScaleMath.SafeRatio(p.Height, 1.0) : 1f;
            s = new Vec3f(sxz, sy, sxz);
            return true;
        }
    }

    public sealed class PrismScaler : IShapeScaler
    {
        public bool TryScale(SceneNode node, out Vec3f s)
        {
            s = default;
            if (node.Params is not PrismParamsVM p) return false;
            var sxz = p.Radius > 0 ? ScaleMath.SafeRatio(p.Radius, 0.5) : 1f;
            var sy = p.Height > 0 ? ScaleMath.SafeRatio(p.Height, 1.0) : 1f;
            s = new Vec3f(sxz, sy, sxz);
            return true;
        }
    }

    public sealed class CircleScaler : IShapeScaler
    {
        public bool TryScale(SceneNode node, out Vec3f s)
        {
            s = default;
            if (node.Params is not CircleParamsVM p) return false;
            var k = p.Radius > 0 ? ScaleMath.SafeRatio(p.Radius, 0.5) : 1f;
            s = new Vec3f(k, 1f, k);
            return true;
        }
    }

    public sealed class TorusScaler : IShapeScaler
    {
        public bool TryScale(SceneNode node, out Vec3f s)
        {
            s = default;
            if (node.Params is not TorusParamsVM p) return false;
            var sxz = ScaleMath.SafeRatio(p.MajorRadius + p.MinorRadius, 0.7);
            var sy = p.MinorRadius > 0 ? ScaleMath.SafeRatio(p.MinorRadius, 0.2) : 1f;
            s = new Vec3f(ScaleMath.ClampPos(sxz), ScaleMath.ClampPos(sy), ScaleMath.ClampPos(sxz));
            return true;
        }
    }
}
