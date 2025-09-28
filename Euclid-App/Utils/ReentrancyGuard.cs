using System;

namespace EuclidApp.Utils
{
    public sealed class ReentrancyGuard
    {
        private int _count;
        public bool IsEntered => _count > 0;

        public IDisposable Enter()
        {
            _count++;
            return new Exit(this);
        }

        private sealed class Exit : IDisposable
        {
            private ReentrancyGuard _g;
            private bool _disposed;

            public Exit(ReentrancyGuard g) => _g = g;

            public void Dispose()
            {
                if (_disposed) return;
                _disposed = true;
                _g._count--;
            }
        }
    }
}
