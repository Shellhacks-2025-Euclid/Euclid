using EuclidApp.Interop;
using EuclidApp.Models;
using EuclidApp.Models.Shapes;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Globalization;

namespace EuclidApp.Utils
{
    public static class SceneGraphUtils
    {
        public static void UnhookAll(ObservableCollection<SceneNode> roots)
        {
            foreach (var r in roots)
                UnhookAll(r);
        }

        public static void UnhookAll(IEnumerable<SceneNode> nodes)
        {
            foreach (var n in nodes)
                UnhookAll(n);
        }

        public static void UnhookAll(SceneNode node)
        {
            if (node.Params is not null)
                node.Params.PropertyChanged -= node.__paramsHandler;

            node.Transform.PropertyChanged -= node.__transformHandler;

            foreach (var c in node.Children)
                UnhookAll(c);
        }

        public static string BuildParamKey(SceneNode n)
        {
            var inv = CultureInfo.InvariantCulture;
            return n.ShapeType switch
            {
                EuclidShapeType.EUCLID_SHAPE_CUBE when n.Params is CubeParamsVM c => $"CUBE|{c.Edge.ToString(inv)}",
                EuclidShapeType.EUCLID_SHAPE_SPHERE when n.Params is SphereParamsVM s => $"SPH|{s.Radius.ToString(inv)}|{s.Slices}|{s.Stacks}",
                EuclidShapeType.EUCLID_SHAPE_PLANE when n.Params is PlaneParamsVM p => $"PLN|{p.Width.ToString(inv)}|{p.Height.ToString(inv)}",
                EuclidShapeType.EUCLID_SHAPE_CONE when n.Params is ConeParamsVM co => $"CON|{co.Radius.ToString(inv)}|{co.Height.ToString(inv)}|{co.Segments}",
                EuclidShapeType.EUCLID_SHAPE_CYLINDER when n.Params is CylinderParamsVM cy => $"CYL|{cy.Radius.ToString(inv)}|{cy.Height.ToString(inv)}|{cy.Segments}",
                EuclidShapeType.EUCLID_SHAPE_PRISM when n.Params is PrismParamsVM pr => $"PRS|{pr.Sides}|{pr.Radius.ToString(inv)}|{pr.Height.ToString(inv)}",
                EuclidShapeType.EUCLID_SHAPE_CIRCLE when n.Params is CircleParamsVM ci => $"CRC|{ci.Radius.ToString(inv)}|{ci.Segments}",
                EuclidShapeType.EUCLID_SHAPE_TORUS when n.Params is TorusParamsVM t => $"TOR|{t.MajorRadius.ToString(inv)}|{t.MinorRadius.ToString(inv)}|{t.SegU}|{t.SegV}",
                _ => $"{n.ShapeType}|noparams"
            };
        }
    }
}
