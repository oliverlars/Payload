import ctypes
import numpy.ctypeslib as ctl
embree3 = ctypes.CDLL('embree3.dll')
Payload= ctypes.CDLL('libpayload.dll')
Payload.Render.restype = ctl.ndpointer(dtype=ctypes.c_float, shape=(1280*720*3,))
Res = Payload.Render()
print("\n")
print(Res)