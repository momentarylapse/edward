# created by xmake

cmake_minimum_required(VERSION 3.21)
project(edward)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)


#=======================================================================================
#    libraries
#=======================================================================================

find_package(PkgConfig REQUIRED)



# lib: gtk4 (mandatory)
pkg_check_modules(GTK4 REQUIRED gtk4)
list(APPEND INCLUDE_DIRECTORIES ${GTK4_INCLUDE_DIRS})
list(APPEND LINK_DIRECTORIES ${GTK4_LIBRARY_DIRS})
list(APPEND DEPS ${GTK4_LIBRARIES})
list(APPEND COMPILE_OPTIONS ${GTK4_CFLAGS_OTHER})
list(APPEND COMPILE_DEFINITIONS HAS_LIB_GTK4=1)


# lib: zlib (mandatory)
find_package(ZLIB REQUIRED)
list(APPEND DEPS ZLIB::ZLIB)
list(APPEND COMPILE_DEFINITIONS HAS_LIB_ZLIB=1)


# lib: gl or vulkan (mandatory)
set(GL_OR_VULKAN "gl" CACHE STRING "Use gl or vulkan?")
find_package(OpenGL)
if(OPENGL_FOUND AND NOT APPLE AND NOT WIN32)
	list(APPEND GL_OR_VULKAN_FOUND "gl")
endif()
pkg_check_modules(VULKAN vulkan)
if(VULKAN_FOUND)
	list(APPEND GL_OR_VULKAN_FOUND "vulkan")
endif()
if(GL_OR_VULKAN_FOUND)
	if(NOT ${GL_OR_VULKAN} IN_LIST GL_OR_VULKAN_FOUND)
		list(GET GL_OR_VULKAN_FOUND 0 X)
		set(GL_OR_VULKAN ${X} CACHE STRING "choose" FORCE)
		message(WARNING "Choice for GL_OR_VULKAN invalid, using ${GL_OR_VULKAN} instead")
	endif()
else()
	message(SEND_ERROR "Neither gl or vulkan found") #  or/nor..
	set(GL_OR_VULKAN "")
endif()
if(GL_OR_VULKAN STREQUAL "gl")
	list(APPEND DEPS OpenGL::OpenGL)
	list(APPEND COMPILE_DEFINITIONS HAS_LIB_GL=1)
endif()
if(GL_OR_VULKAN STREQUAL "vulkan")
	list(APPEND INCLUDE_DIRECTORIES ${VULKAN_INCLUDE_DIRS})
	list(APPEND LINK_DIRECTORIES ${VULKAN_LIBRARY_DIRS})
	list(APPEND DEPS ${VULKAN_LIBRARIES})
	list(APPEND COMPILE_OPTIONS ${VULKAN_CFLAGS_OTHER})
	list(APPEND COMPILE_DEFINITIONS HAS_LIB_VULKAN=1)
endif()


# lib: glew (optional)
set(LIB_GLEW_ENABLED true CACHE BOOL "Use glew library?")
pkg_check_modules(GLEW glew)
if(GLEW_FOUND)
else()
	set(LIB_GLEW_ENABLED false CACHE BOOL "glew library not found" FORCE)
endif()
if(${LIB_GLEW_ENABLED})
	list(APPEND INCLUDE_DIRECTORIES ${GLEW_INCLUDE_DIRS})
	list(APPEND LINK_DIRECTORIES ${GLEW_LIBRARY_DIRS})
	list(APPEND DEPS ${GLEW_LIBRARIES})
	list(APPEND COMPILE_OPTIONS ${GLEW_CFLAGS_OTHER})
	list(APPEND COMPILE_DEFINITIONS HAS_LIB_GLEW=1)
else()
	list(APPEND COMPILE_DEFINITIONS HAS_LIB_GLEW=0)
endif()


# lib: glfw (optional)
set(LIB_GLFW_ENABLED true CACHE BOOL "Use glfw library?")
find_package(glfw3)
if(glfw3_FOUND)
else()
	set(LIB_GLFW_ENABLED false CACHE BOOL "glfw library not found" FORCE)
endif()
if(${LIB_GLFW_ENABLED})
	list(APPEND DEPS glfw)
	list(APPEND COMPILE_DEFINITIONS HAS_LIB_GLFW=1)
else()
	list(APPEND COMPILE_DEFINITIONS HAS_LIB_GLFW=0)
endif()


# lib: shaderc (optional)
set(LIB_SHADERC_ENABLED true CACHE BOOL "Use shaderc library?")
pkg_check_modules(SHADERC shaderc)
if(SHADERC_FOUND)
else()
	set(LIB_SHADERC_ENABLED false CACHE BOOL "shaderc library not found" FORCE)
endif()
if(${LIB_SHADERC_ENABLED})
	list(APPEND INCLUDE_DIRECTORIES ${SHADERC_INCLUDE_DIRS})
	list(APPEND LINK_DIRECTORIES ${SHADERC_LIBRARY_DIRS})
	list(APPEND DEPS ${SHADERC_LIBRARIES})
	list(APPEND COMPILE_OPTIONS ${SHADERC_CFLAGS_OTHER})
	list(APPEND COMPILE_DEFINITIONS HAS_LIB_SHADERC=1)
else()
	list(APPEND COMPILE_DEFINITIONS HAS_LIB_SHADERC=0)
endif()


# lib: spirv (optional)
set(LIB_SPIRV_ENABLED true CACHE BOOL "Use spirv library?")
pkg_check_modules(SPIRV spirv)
if(SPIRV_FOUND)
else()
	set(LIB_SPIRV_ENABLED false CACHE BOOL "spirv library not found" FORCE)
endif()
if(${LIB_SPIRV_ENABLED})
	list(APPEND INCLUDE_DIRECTORIES ${SPIRV_INCLUDE_DIRS})
	list(APPEND LINK_DIRECTORIES ${SPIRV_LIBRARY_DIRS})
	list(APPEND DEPS ${SPIRV_LIBRARIES})
	list(APPEND COMPILE_OPTIONS ${SPIRV_CFLAGS_OTHER})
	list(APPEND COMPILE_DEFINITIONS HAS_LIB_SPIRV=1)
else()
	list(APPEND COMPILE_DEFINITIONS HAS_LIB_SPIRV=0)
endif()


# lib: unwind (optional)
set(LIB_UNWIND_ENABLED true CACHE BOOL "Use unwind library?")
pkg_check_modules(LIBUNWIND libunwind)
if(LIBUNWIND_FOUND)
else()
	set(LIB_UNWIND_ENABLED false CACHE BOOL "unwind library not found" FORCE)
endif()
if(${LIB_UNWIND_ENABLED})
	list(APPEND INCLUDE_DIRECTORIES ${LIBUNWIND_INCLUDE_DIRS})
	list(APPEND LINK_DIRECTORIES ${LIBUNWIND_LIBRARY_DIRS})
	list(APPEND DEPS ${LIBUNWIND_LIBRARIES})
	list(APPEND COMPILE_OPTIONS ${LIBUNWIND_CFLAGS_OTHER})
	list(APPEND COMPILE_DEFINITIONS HAS_LIB_UNWIND=1)
else()
	list(APPEND COMPILE_DEFINITIONS HAS_LIB_UNWIND=0)
endif()


# lib: dl (optional)
if(WIN32)
	list(APPEND COMPILE_DEFINITIONS HAS_LIB_DL=0)
else()
	list(APPEND DEPS ${CMAKE_DL_LIBS})
	list(APPEND COMPILE_DEFINITIONS HAS_LIB_DL=1)
endif()



# lib: xlib (optional)
set(LIB_XLIB_ENABLED true CACHE BOOL "Use xlib library?")
pkg_check_modules(X11 x11)
if(X11_FOUND)
else()
	set(LIB_XLIB_ENABLED false CACHE BOOL "xlib library not found" FORCE)
endif()
if(${LIB_XLIB_ENABLED})
	list(APPEND INCLUDE_DIRECTORIES ${X11_INCLUDE_DIRS})
	list(APPEND LINK_DIRECTORIES ${X11_LIBRARY_DIRS})
	list(APPEND DEPS ${X11_LIBRARIES})
	list(APPEND COMPILE_OPTIONS ${X11_CFLAGS_OTHER})
	list(APPEND COMPILE_DEFINITIONS HAS_LIB_XLIB=1)
else()
	list(APPEND COMPILE_DEFINITIONS HAS_LIB_XLIB=0)
endif()


# lib: freetype2 (mandatory)
find_package(Freetype REQUIRED)
list(APPEND DEPS Freetype::Freetype)
list(APPEND COMPILE_DEFINITIONS HAS_LIB_FREETYPE2=1)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
list(APPEND DEPS Threads::Threads)


#=======================================================================================
#    target edward
#=======================================================================================

add_executable(edward
	src/action/Action.cpp
	src/action/ActionGroup.cpp
	src/action/ActionManager.cpp
	src/action/ActionMultiView.cpp
	src/data/geometry/Geometry.cpp
	src/data/geometry/GeometryBall.cpp
	src/data/geometry/GeometryCube.cpp
	src/data/geometry/GeometryCylinder.cpp
	src/data/geometry/GeometryPlane.cpp
	src/data/geometry/GeometryPlatonic.cpp
	src/data/geometry/GeometrySphere.cpp
	src/data/geometry/GeometrySubtract.cpp
	src/data/geometry/GeometryTeapot.cpp
	src/data/geometry/GeometryTorus.cpp
	src/data/geometry/GeometryTorusKnot.cpp
	src/data/Data.cpp
	src/lib/any/any.cpp
	src/lib/base/array.cpp
	src/lib/base/pointer.cpp
	src/lib/base/strings.cpp
	src/lib/doc/chunked.cpp
	src/lib/doc/pdf.cpp
	src/lib/doc/ttf.cpp
	src/lib/doc/xml.cpp
	src/lib/image/color.cpp
	src/lib/image/image.cpp
	src/lib/image/image_bmp.cpp
	src/lib/image/image_jpg.cpp
	src/lib/image/image_png.cpp
	src/lib/image/image_tga.cpp
	src/lib/image/ImagePainter.cpp
	src/lib/kaba/asm/asm.cpp
	src/lib/kaba/asm/asm_arm.cpp
	src/lib/kaba/asm/asm_x86.cpp
	src/lib/kaba/compiler/Backend.cpp
	src/lib/kaba/compiler/BackendAmd64.cpp
	src/lib/kaba/compiler/BackendARM.cpp
	src/lib/kaba/compiler/BackendArm64.cpp
	src/lib/kaba/compiler/BackendX86.cpp
	src/lib/kaba/compiler/CommandList.cpp
	src/lib/kaba/compiler/Compiler.cpp
	src/lib/kaba/compiler/mapper.cpp
	src/lib/kaba/compiler/Serializer.cpp
	src/lib/kaba/compiler/SerialNode.cpp
	src/lib/kaba/dynamic/call.cpp
	src/lib/kaba/dynamic/dynamic.cpp
	src/lib/kaba/dynamic/exception.cpp
	src/lib/kaba/dynamic/sorting.cpp
	src/lib/kaba/lib/dict.cpp
	src/lib/kaba/lib/extern.cpp
	src/lib/kaba/lib/lib.cpp
	src/lib/kaba/lib/lib_async.cpp
	src/lib/kaba/lib/lib_base.cpp
	src/lib/kaba/lib/lib_doc.cpp
	src/lib/kaba/lib/lib_gl.cpp
	src/lib/kaba/lib/lib_hui.cpp
	src/lib/kaba/lib/lib_image.cpp
	src/lib/kaba/lib/lib_kaba.cpp
	src/lib/kaba/lib/lib_math.cpp
	src/lib/kaba/lib/lib_net.cpp
	src/lib/kaba/lib/lib_os.cpp
	src/lib/kaba/lib/lib_threads.cpp
	src/lib/kaba/lib/lib_time.cpp
	src/lib/kaba/lib/lib_vulkan.cpp
	src/lib/kaba/lib/list.cpp
	src/lib/kaba/parser/abstract.cpp
	src/lib/kaba/parser/Concretifier.cpp
	src/lib/kaba/parser/import.cpp
	src/lib/kaba/parser/lexical.cpp
	src/lib/kaba/parser/macros.cpp
	src/lib/kaba/parser/Parser.cpp
	src/lib/kaba/parser/Transformer.cpp
	src/lib/kaba/parser/type_matching.cpp
	src/lib/kaba/syntax/Block.cpp
	src/lib/kaba/syntax/Class.cpp
	src/lib/kaba/syntax/Constant.cpp
	src/lib/kaba/syntax/Function.cpp
	src/lib/kaba/syntax/Identifier.cpp
	src/lib/kaba/syntax/Node.cpp
	src/lib/kaba/syntax/Operator.cpp
	src/lib/kaba/syntax/preprocessor.cpp
	src/lib/kaba/syntax/Statement.cpp
	src/lib/kaba/syntax/SyntaxTree.cpp
	src/lib/kaba/syntax/Variable.cpp
	src/lib/kaba/template/implicit.cpp
	src/lib/kaba/template/implicit_array.cpp
	src/lib/kaba/template/implicit_callable.cpp
	src/lib/kaba/template/implicit_dict.cpp
	src/lib/kaba/template/implicit_enum.cpp
	src/lib/kaba/template/implicit_future.cpp
	src/lib/kaba/template/implicit_list.cpp
	src/lib/kaba/template/implicit_optional.cpp
	src/lib/kaba/template/implicit_pointer.cpp
	src/lib/kaba/template/implicit_product.cpp
	src/lib/kaba/template/implicit_regular.cpp
	src/lib/kaba/template/template.cpp
	src/lib/kaba/CompilerConfiguration.cpp
	src/lib/kaba/Context.cpp
	src/lib/kaba/Interpreter.cpp
	src/lib/kaba/kaba.cpp
	src/lib/kaba/Module.cpp
	src/lib/math/Box.cpp
	src/lib/math/complex.cpp
	src/lib/math/interpolation.cpp
	src/lib/math/mat3.cpp
	src/lib/math/mat4.cpp
	src/lib/math/math.cpp
	src/lib/math/plane.cpp
	src/lib/math/quaternion.cpp
	src/lib/math/random.cpp
	src/lib/math/ray.cpp
	src/lib/math/rect.cpp
	src/lib/math/vec2.cpp
	src/lib/math/vec3.cpp
	src/lib/math/vec4.cpp
	src/lib/net/BinaryBuffer.cpp
	src/lib/net/net.cpp
	src/lib/net/Socket.cpp
	src/lib/nix/nix.cpp
	src/lib/nix/nix_buffer.cpp
	src/lib/nix/nix_draw.cpp
	src/lib/nix/nix_framebuffer.cpp
	src/lib/nix/nix_light.cpp
	src/lib/nix/nix_shader.cpp
	src/lib/nix/nix_textures.cpp
	src/lib/nix/nix_vertexbuffer.cpp
	src/lib/nix/nix_view.cpp
	src/lib/os/CommandLineParser.cpp
	src/lib/os/config.cpp
	src/lib/os/date.cpp
	src/lib/os/file.cpp
	src/lib/os/filesystem.cpp
	src/lib/os/formatter.cpp
	src/lib/os/msg.cpp
	src/lib/os/path.cpp
	src/lib/os/stream.cpp
	src/lib/os/terminal.cpp
	src/lib/os/time.cpp
	src/lib/pattern/Observable.cpp
	src/lib/threads/Mutex.cpp
	src/lib/threads/Thread.cpp
	src/lib/threads/ThreadedWork.cpp
	src/lib/vulkan/AccelerationStructure.cpp
	src/lib/vulkan/Buffer.cpp
	src/lib/vulkan/CommandBuffer.cpp
	src/lib/vulkan/DescriptorSet.cpp
	src/lib/vulkan/Device.cpp
	src/lib/vulkan/FrameBuffer.cpp
	src/lib/vulkan/helper.cpp
	src/lib/vulkan/Instance.cpp
	src/lib/vulkan/Pipeline.cpp
	src/lib/vulkan/Queue.cpp
	src/lib/vulkan/RenderPass.cpp
	src/lib/vulkan/Semaphore.cpp
	src/lib/vulkan/Shader.cpp
	src/lib/vulkan/SwapChain.cpp
	src/lib/vulkan/Texture.cpp
	src/lib/vulkan/VertexBuffer.cpp
	src/lib/vulkan/vulkan.cpp
	src/lib/xhui/controls/Button.cpp
	src/lib/xhui/controls/CheckBox.cpp
	src/lib/xhui/controls/ColorButton.cpp
	src/lib/xhui/controls/Control.cpp
	src/lib/xhui/controls/DrawingArea.cpp
	src/lib/xhui/controls/Edit.cpp
	src/lib/xhui/controls/FileSelector.cpp
	src/lib/xhui/controls/Grid.cpp
	src/lib/xhui/controls/Group.cpp
	src/lib/xhui/controls/HeaderBar.cpp
	src/lib/xhui/controls/Label.cpp
	src/lib/xhui/controls/ListView.cpp
	src/lib/xhui/controls/MultilineEdit.cpp
	src/lib/xhui/controls/Overlay.cpp
	src/lib/xhui/controls/SpinButton.cpp
	src/lib/xhui/controls/TabControl.cpp
	src/lib/xhui/controls/Toolbar.cpp
	src/lib/xhui/dialogs/ColorSelectionDialog.cpp
	src/lib/xhui/dialogs/FileSelectionDialog.cpp
	src/lib/xhui/dialogs/QuestionDialog.cpp
	src/lib/xhui/draw/font.cpp
	src/lib/xhui/ContextVulkan.cpp
	src/lib/xhui/Dialog.cpp
	src/lib/xhui/language.cpp
	src/lib/xhui/Menu.cpp
	src/lib/xhui/PainterGL.cpp
	src/lib/xhui/PainterVulkan.cpp
	src/lib/xhui/Panel.cpp
	src/lib/xhui/Resource.cpp
	src/lib/xhui/Theme.cpp
	src/lib/xhui/Window.cpp
	src/lib/xhui/xhui.cpp
	src/mode_model/data/DataModel.cpp
	src/mode_model/data/ModelMaterial.cpp
	src/mode_model/data/ModelMesh.cpp
	src/mode_model/data/ModelPolygon.cpp
	src/mode_model/data/SkinGenerator.cpp
	src/mode_model/ModeAddVertexPolygon.cpp
	src/mode_model/ModeModel.cpp
	src/mode_world/action/entity/ActionWorldAddEntity.cpp
	src/mode_world/action/entity/ActionWorldEditComponent.cpp
	src/mode_world/action/light/ActionWorldAddLight.cpp
	src/mode_world/action/object/ActionWorldAddObject.cpp
	src/mode_world/action/object/ActionWorldDeleteObject.cpp
	src/mode_world/action/terrain/ActionWorldAddTerrain.cpp
	src/mode_world/action/ActionWorldDeleteSelection.cpp
	src/mode_world/action/ActionWorldEditData.cpp
	src/mode_world/action/ActionWorldMoveSelection.cpp
	src/mode_world/action/ActionWorldPaste.cpp
	src/mode_world/data/DataCamera.cpp
	src/mode_world/data/DataWorld.cpp
	src/mode_world/data/WorldCamera.cpp
	src/mode_world/data/WorldLight.cpp
	src/mode_world/data/WorldLink.cpp
	src/mode_world/data/WorldObject.cpp
	src/mode_world/data/WorldTerrain.cpp
	src/mode_world/dialog/PropertiesDialog.cpp
	src/mode_world/AddEntityPanel.cpp
	src/mode_world/EntityPanel.cpp
	src/mode_world/ModeAddEntity.cpp
	src/mode_world/ModeWorld.cpp
	src/multiview/SingleData.cpp
	src/storage/format/old/LoadWorldV9V10.cpp
	src/storage/format/Format.cpp
	src/storage/format/FormatModel.cpp
	src/storage/format/FormatModel3ds.cpp
	src/storage/format/FormatModelJson.cpp
	src/storage/format/FormatModelPly.cpp
	src/storage/format/FormatTerrain.cpp
	src/storage/format/FormatWorld.cpp
	src/storage/Storage.cpp
	src/view/ActionController.cpp
	src/view/DrawingHelper.cpp
	src/view/EdwardWindow.cpp
	src/view/Mode.cpp
	src/view/MultiView.cpp
	src/y/audio/audio.cpp
	src/y/audio/AudioBuffer.cpp
	src/y/audio/AudioStream.cpp
	src/y/audio/Listener.cpp
	src/y/audio/Loading.cpp
	src/y/audio/SoundSource.cpp
	src/y/fx/Beam.cpp
	src/y/fx/Particle.cpp
	src/y/fx/ParticleEmitter.cpp
	src/y/fx/ParticleManager.cpp
	src/y/gui/Font.cpp
	src/y/gui/gui.cpp
	src/y/gui/Node.cpp
	src/y/gui/Picture.cpp
	src/y/gui/Text.cpp
	src/y/helper/DeletionQueue.cpp
	src/y/helper/ErrorHandler.cpp
	src/y/helper/PerformanceMonitor.cpp
	src/y/helper/ResourceManager.cpp
	src/y/helper/Scheduler.cpp
	src/y/net/NetworkManager.cpp
	src/y/plugins/Controller.cpp
	src/y/plugins/ControllerManager.cpp
	src/y/plugins/PluginManager.cpp
	src/y/renderer/gui/GuiRendererGL.cpp
	src/y/renderer/gui/GuiRendererVulkan.cpp
	src/y/renderer/helper/Bindable.cpp
	src/y/renderer/helper/ComputeTask.cpp
	src/y/renderer/helper/CubeMapSource.cpp
	src/y/renderer/helper/jitter.cpp
	src/y/renderer/helper/LightMeter.cpp
	src/y/renderer/helper/Pipeline.cpp
	src/y/renderer/helper/PipelineManager.cpp
	src/y/renderer/helper/Raytracing.cpp
	src/y/renderer/helper/RendererFactory.cpp
	src/y/renderer/path/RenderPath.cpp
	src/y/renderer/path/RenderPathDirect.cpp
	src/y/renderer/post/HDRResolver.cpp
	src/y/renderer/post/MultisampleResolver.cpp
	src/y/renderer/post/PostProcessor.cpp
	src/y/renderer/post/PostProcessorGL.cpp
	src/y/renderer/post/PostProcessorVulkan.cpp
	src/y/renderer/post/ThroughShaderRenderer.cpp
	src/y/renderer/regions/RegionRenderer.cpp
	src/y/renderer/regions/RegionRendererGL.cpp
	src/y/renderer/regions/RegionRendererVulkan.cpp
	src/y/renderer/target/HeadlessRendererVulkan.cpp
	src/y/renderer/target/HuiWindowRendererGL.cpp
	src/y/renderer/target/HuiWindowRendererVulkan.cpp
	src/y/renderer/target/TargetRenderer.cpp
	src/y/renderer/target/TextureRendererGL.cpp
	src/y/renderer/target/TextureRendererVulkan.cpp
	src/y/renderer/target/WindowRendererGL.cpp
	src/y/renderer/target/WindowRendererVulkan.cpp
	src/y/renderer/target/XhuiRenderer.cpp
	src/y/renderer/world/geometry/GeometryRenderer.cpp
	src/y/renderer/world/geometry/GeometryRendererGL.cpp
	src/y/renderer/world/geometry/GeometryRendererVulkan.cpp
	src/y/renderer/world/geometry/RenderViewData.cpp
	src/y/renderer/world/geometry/SceneView.cpp
	src/y/renderer/world/pass/ShadowRenderer.cpp
	src/y/renderer/world/WorldRenderer.cpp
	src/y/renderer/world/WorldRendererDeferred.cpp
	src/y/renderer/world/WorldRendererForward.cpp
	src/y/renderer/world/WorldRendererVulkanRayTracing.cpp
	src/y/renderer/base.cpp
	src/y/renderer/Renderer.cpp
	src/y/world/components/Animator.cpp
	src/y/world/components/Collider.cpp
	src/y/world/components/MultiInstance.cpp
	src/y/world/components/Skeleton.cpp
	src/y/world/components/SolidBody.cpp
	src/y/world/components/UserMesh.cpp
	src/y/world/Camera.cpp
	src/y/world/LevelData.cpp
	src/y/world/Light.cpp
	src/y/world/Link.cpp
	src/y/world/Material.cpp
	src/y/world/Model.cpp
	src/y/world/ModelManager.cpp
	src/y/world/Terrain.cpp
	src/y/world/World.cpp
	src/y/y/BaseClass.cpp
	src/y/y/Component.cpp
	src/y/y/ComponentManager.cpp
	src/y/y/EngineData.cpp
	src/y/y/Entity.cpp
	src/y/Config.cpp
	src/Edward.cpp
	src/Session.cpp)
target_include_directories(edward PUBLIC ${INCLUDE_DIRECTORIES})
target_link_directories(edward PUBLIC ${LINK_DIRECTORIES})
target_compile_options(edward PUBLIC ${COMPILE_OPTIONS})
target_compile_definitions(edward PUBLIC ${COMPILE_DEFINITIONS} INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}")
target_link_libraries(edward PUBLIC ${DEPS})
target_include_directories(edward PUBLIC src)
target_include_directories(edward PUBLIC src/y)


#=======================================================================================
#    install
#=======================================================================================

set(RESOURCE_DESTINATION share/edward)

# binaries
install(TARGETS edward DESTINATION bin)	

# resources
install(DIRECTORY "static/icons" DESTINATION ${RESOURCE_DESTINATION})
install(DIRECTORY "static/shader" DESTINATION ${RESOURCE_DESTINATION})
install(FILES "static/edward.jpg" DESTINATION ${RESOURCE_DESTINATION})
install(FILES "static/hui_resources.txt" DESTINATION ${RESOURCE_DESTINATION})
install(FILES "static/icon.svg" DESTINATION ${RESOURCE_DESTINATION})
install(FILES "static/keys.txt" DESTINATION ${RESOURCE_DESTINATION})
install(FILES "static/license.txt" DESTINATION ${RESOURCE_DESTINATION})
install(FILES "static/license_small.txt" DESTINATION ${RESOURCE_DESTINATION})
install(FILES "static/OpenSans-Regular.ttf" DESTINATION ${RESOURCE_DESTINATION})
install(DIRECTORY "plugins" DESTINATION ${RESOURCE_DESTINATION})


if (UNIX AND NOT APPLE)
	# install .desktop file
	set(INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
	configure_file("${CMAKE_CURRENT_SOURCE_DIR}/static/michisoft-edward.desktop" /tmp/michisoft-edward.desktop NO_SOURCE_PERMISSIONS)
	install(FILES /tmp/michisoft-edward.desktop DESTINATION share/applications)
endif ()

if (UNIX AND NOT APPLE)
	# install mime
	find_program(XDG_MIME_EXECUTABLE xdg-mime)
	if(NOT XDG_MIME_EXECUTABLE STREQUAL "XDG_MIME_EXECUTABLE_NOTFOUND")
		install(CODE "message(STATUS \"installing mime file\")")
		install(CODE "execute_process(COMMAND ${XDG_MIME_EXECUTABLE} install --novendor ${CMAKE_CURRENT_SOURCE_DIR}/static/michisoft-x-model.xml)")
		install(CODE "message(STATUS \"installing mime file\")")
		install(CODE "execute_process(COMMAND ${XDG_MIME_EXECUTABLE} install --novendor ${CMAKE_CURRENT_SOURCE_DIR}/static/michisoft-x-font.xml)")
		install(CODE "message(STATUS \"installing mime file\")")
		install(CODE "execute_process(COMMAND ${XDG_MIME_EXECUTABLE} install --novendor ${CMAKE_CURRENT_SOURCE_DIR}/static/michisoft-x-material.xml)")
		install(CODE "message(STATUS \"installing mime file\")")
		install(CODE "execute_process(COMMAND ${XDG_MIME_EXECUTABLE} install --novendor ${CMAKE_CURRENT_SOURCE_DIR}/static/michisoft-x-world.xml)")
		install(CODE "message(STATUS \"installing mime file\")")
		install(CODE "execute_process(COMMAND ${XDG_MIME_EXECUTABLE} install --novendor ${CMAKE_CURRENT_SOURCE_DIR}/static/michisoft-x-terrain.xml)")
	endif()
endif()
