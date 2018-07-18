import bpy
import array
import numpy as np
import bgl, bl_ui
import ctypes
import numpy.ctypeslib as ctl
import os

print(os.listdir("C:/Users/Oliver/Documents/Programming/CPP/Payload/build/out/"))
DLLPath = r'c:/Users/Oliver/Documents/Programming/CPP/Payload/build/out/libpayload.dll'
print(os.path.exists(DLLPath))
embree3 = ctypes.WinDLL("C:/Users/Oliver/Documents/Programming/CPP/Payload/build/out/embree3.dll")
Payload = ctypes.WinDLL("C:/Users/Oliver/Documents/Programming/CPP/Payload/build/out/libpayload.dll")
Payload.Render.restype = ctl.ndpointer(dtype=ctypes.c_float, shape=(1280*720*3,))
Res = Payload.Render()
print(Res)
class CustomRenderEngine(bpy.types.RenderEngine):
    # These three members are used by blender to set up the
    # RenderEngine; define its internal name, visible name and capabilities.
    bl_idname = "custom_renderer"
    bl_label = "Payload Render"
    bl_use_preview = True
    bufferfloat = array.array('f', [])
    npfloat = []
    glbuffer = None
    width = 0
    height = 0
    # This is the only method called by blender, in this example
    # we use it to detect preview rendering and call the implementation
    # in another method.
    def render(self, scene):
        scale = scene.render.resolution_percentage / 100.0
        self.size_x = int(scene.render.resolution_x * scale)
        self.size_y = int(scene.render.resolution_y * scale)

        if self.is_preview:
            self.render_preview(scene)
        else:
            self.render_scene(scene)
            
    def create_solid_colour_buffer(self, width, height):
        self.bufferfloat = array.array('f', [0.0, 1.0, 0.0]*self.width*self.height)
        
    # In this example, we fill the preview renders with a flat green color.
    def render_preview(self, scene):
        pixel_count = self.size_x * self.size_y

        # The framebuffer is defined as a list of pixels, each pixel
        # itself being a list of R,G,B,A values
        green_rect = [[0.0, 1.0, 0.0, 1.0]] * pixel_count

        # Here we write the pixel values to the RenderResult
        result = self.begin_result(0, 0, self.size_x, self.size_y)
        layer = result.layers[0].passes["Combined"]
        layer.rect = green_rect
        self.end_result(result)
        
    def view_update(self, context):
        self.width = context.region.width
        self.height = context.region.height
        bufferdepth = 3
        buffersize = self.width*self.height*bufferdepth
        self.npfloat = np.random.rand(self.width*self.height*3)
        print(self.width)
        print(self.height)
        self.glbuffer = bgl.Buffer(bgl.GL_FLOAT, [buffersize], self.npfloat)
        #self.create_solid_colour_buffer(context.region.width, context.region.height)
        
    def view_draw(self, context):
        bgl.glRasterPos2i(0,0)
        bgl.glDrawPixels(self.width, self.height, bgl.GL_RGB, bgl.GL_FLOAT, self.glbuffer)
        bgl.glDisable(bgl.GL_BLEND)
    # In this example, we fill the full renders with a flat blue color.
    def render_scene(self, scene):
        pixel_count = self.size_x * self.size_y

        # The framebuffer is defined as a list of pixels, each pixel
        # itself being a list of R,G,B,A values
        blue_rect = [[0.0, 0.0, 1.0, 1.0]] * pixel_count
                
        # Here we write the pixel values to the RenderResult
        result = self.begin_result(0, 0, self.size_x, self.size_y)
        layer = result.layers[0].passes["Combined"]
        layer.rect = blue_rect
        self.end_result(result)


def register():
    # Register the RenderEngine
    bpy.utils.register_class(CustomRenderEngine)

    # RenderEngines also need to tell UI Panels that they are compatible
    # Otherwise most of the UI will be empty when the engine is selected.
    # In this example, we need to see the main render image button and
    # the material preview panel.
    from bl_ui import (
            properties_render,
            properties_material,
            )
    properties_render.RENDER_PT_render.COMPAT_ENGINES.add(CustomRenderEngine.bl_idname)
    properties_material.MATERIAL_PT_preview.COMPAT_ENGINES.add(CustomRenderEngine.bl_idname)


def unregister():
    bpy.utils.unregister_class(CustomRenderEngine)

    from bl_ui import (
            properties_render,
            properties_material,
            )
    properties_render.RENDER_PT_render.COMPAT_ENGINES.remove(CustomRenderEngine.bl_idname)
    properties_material.MATERIAL_PT_preview.COMPAT_ENGINES.remove(CustomRenderEngine.bl_idname)


if __name__ == "__main__":
    register()