namespace EuclidApp.Utils
{
    public static class ScaleMath
    {
        public const float Eps = 1e-5f;
        public static float ClampPos(float v, float min = 0.0001f) => v < min ? min : v;

        public static float SafeRatio(double num, double den, double fallback = 1.0)
        {
            if (double.IsNaN(num) || double.IsNaN(den) || Math.Abs(den) < double.Epsilon) return (float)fallback;
            return (float)(num / den);
        }

        public static bool NearlyEqual(float a, float b, float eps = Eps) => Math.Abs(a - b) <= eps;
    }

    public readonly struct Vec3f
    {
        public readonly float X, Y, Z;
        public Vec3f(float x, float y, float z) { X = x; Y = y; Z = z; }

        public bool NearlyEquals(in Vec3f other, float eps = ScaleMath.Eps) =>
            ScaleMath.NearlyEqual(X, other.X, eps) &&
            ScaleMath.NearlyEqual(Y, other.Y, eps) &&
            ScaleMath.NearlyEqual(Z, other.Z, eps);
    }
}
