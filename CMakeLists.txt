# created by xmake

project(edward)
cmake_minimum_required(VERSION 3.22)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)
set(SOURCE 
	src/Action/Font/ActionFontEditGlobal.cpp
	src/Action/Font/ActionFontEditGlyph.cpp
	src/Action/Material/ActionMaterialEditAppearance.cpp
	src/Action/Material/ActionMaterialEditPhysics.cpp
	src/Action/Model/Animation/ActionModelAddAnimation.cpp
	src/Action/Model/Animation/ActionModelAnimationAddFrame.cpp
	src/Action/Model/Animation/ActionModelAnimationDeleteFrame.cpp
	src/Action/Model/Animation/ActionModelAnimationSetBone.cpp
	src/Action/Model/Animation/ActionModelAnimationSetData.cpp
	src/Action/Model/Animation/ActionModelAnimationSetFrameDuration.cpp
	src/Action/Model/Animation/ActionModelAnimationTransformBones.cpp
	src/Action/Model/Animation/ActionModelAnimationTransformVertices.cpp
	src/Action/Model/Animation/ActionModelDeleteAnimation.cpp
	src/Action/Model/Animation/ActionModelDuplicateAnimation.cpp
	src/Action/Model/Data/ActionModelAddMaterial.cpp
	src/Action/Model/Data/ActionModelDeleteMaterial.cpp
	src/Action/Model/Data/ActionModelEditData.cpp
	src/Action/Model/Data/ActionModelEditMaterial.cpp
	src/Action/Model/Mesh/Brush/ActionModelBrushComplexify.cpp
	src/Action/Model/Mesh/Brush/ActionModelBrushExtrude.cpp
	src/Action/Model/Mesh/Brush/ActionModelBrushSmooth.cpp
	src/Action/Model/Mesh/Edge/ActionModelBevelEdges.cpp
	src/Action/Model/Mesh/Edge/ActionModelCollapseEdge.cpp
	src/Action/Model/Mesh/Edge/ActionModelSplitEdge.cpp
	src/Action/Model/Mesh/Effects/ActionModelAddEffects.cpp
	src/Action/Model/Mesh/Effects/ActionModelClearEffects.cpp
	src/Action/Model/Mesh/Effects/ActionModelEditEffect.cpp
	src/Action/Model/Mesh/Look/ActionModelSetMaterial.cpp
	src/Action/Model/Mesh/Look/ActionModelSetNormalModeSelection.cpp
	src/Action/Model/Mesh/Physical/ActionModelAddBall.cpp
	src/Action/Model/Mesh/Physical/ActionModelAddCylinder.cpp
	src/Action/Model/Mesh/Physical/ActionModelDeleteBall.cpp
	src/Action/Model/Mesh/Physical/ActionModelDeleteCylinder.cpp
	src/Action/Model/Mesh/Polygon/Helper/ActionModelPolygonAddVertex.cpp
	src/Action/Model/Mesh/Polygon/Helper/ActionModelPolygonRemoveVertex.cpp
	src/Action/Model/Mesh/Polygon/ActionModelAddPolygon.cpp
	src/Action/Model/Mesh/Polygon/ActionModelAddPolygonAutoSkin.cpp
	src/Action/Model/Mesh/Polygon/ActionModelAddPolygonSingleTexture.cpp
	src/Action/Model/Mesh/Polygon/ActionModelAddPolygonWithSkinGenerator.cpp
	src/Action/Model/Mesh/Polygon/ActionModelCutOutPolygons.cpp
	src/Action/Model/Mesh/Polygon/ActionModelExtrudePolygons.cpp
	src/Action/Model/Mesh/Polygon/ActionModelMergePolygonsSelection.cpp
	src/Action/Model/Mesh/Polygon/ActionModelSplitPolygon.cpp
	src/Action/Model/Mesh/Polygon/ActionModelTriangulateSelection.cpp
	src/Action/Model/Mesh/Skin/ActionModelAutomap.cpp
	src/Action/Model/Mesh/Skin/ActionModelSkinVerticesFromProjection.cpp
	src/Action/Model/Mesh/Skin/ActionModelTransformSkinVertices.cpp
	src/Action/Model/Mesh/Surface/Helper/ActionModelAddEmptySurface.cpp
	src/Action/Model/Mesh/Surface/Helper/ActionModelDeleteEmptySurface.cpp
	src/Action/Model/Mesh/Surface/Helper/ActionModelJoinSurfaces.cpp
	src/Action/Model/Mesh/Surface/Helper/ActionModelSurfaceAddPolygon.cpp
	src/Action/Model/Mesh/Surface/Helper/ActionModelSurfaceDeletePolygon.cpp
	src/Action/Model/Mesh/Surface/Helper/ActionModelSurfaceRelinkPolygon.cpp
	src/Action/Model/Mesh/Surface/ActionModelAutoWeldSelection.cpp
	src/Action/Model/Mesh/Surface/ActionModelDeleteSurface.cpp
	src/Action/Model/Mesh/Surface/ActionModelSurfaceAutoWeld.cpp
	src/Action/Model/Mesh/Surface/ActionModelSurfaceCopy.cpp
	src/Action/Model/Mesh/Surface/ActionModelSurfaceInvert.cpp
	src/Action/Model/Mesh/Surface/ActionModelSurfacesSubdivide.cpp
	src/Action/Model/Mesh/Surface/ActionModelSurfaceVolumeAnd.cpp
	src/Action/Model/Mesh/Surface/ActionModelSurfaceVolumeSubtract.cpp
	src/Action/Model/Mesh/Vertex/Helper/ActionModelDeleteUnusedVertex.cpp
	src/Action/Model/Mesh/Vertex/Helper/ActionModelMoveVertex.cpp
	src/Action/Model/Mesh/Vertex/ActionModelAddVertex.cpp
	src/Action/Model/Mesh/Vertex/ActionModelAlignToGrid.cpp
	src/Action/Model/Mesh/Vertex/ActionModelBevelVertices.cpp
	src/Action/Model/Mesh/Vertex/ActionModelCollapseVertices.cpp
	src/Action/Model/Mesh/Vertex/ActionModelDeleteVertex.cpp
	src/Action/Model/Mesh/Vertex/ActionModelFlattenVertices.cpp
	src/Action/Model/Mesh/Vertex/ActionModelNearifyVertices.cpp
	src/Action/Model/Mesh/Vertex/ActionModelTransformVertices.cpp
	src/Action/Model/Mesh/Vertex/ActionModelTriangulateVertices.cpp
	src/Action/Model/Mesh/ActionModelDeleteSelection.cpp
	src/Action/Model/Mesh/ActionModelEasify.cpp
	src/Action/Model/Mesh/ActionModelPasteGeometry.cpp
	src/Action/Model/Skeleton/ActionModelAddBone.cpp
	src/Action/Model/Skeleton/ActionModelAttachVerticesToBone.cpp
	src/Action/Model/Skeleton/ActionModelDeleteBone.cpp
	src/Action/Model/Skeleton/ActionModelDeleteBoneSelection.cpp
	src/Action/Model/Skeleton/ActionModelReconnectBone.cpp
	src/Action/Model/Skeleton/ActionModelSetSubModel.cpp
	src/Action/Model/Skeleton/ActionModelTransformBones.cpp
	src/Action/World/Camera/ActionCameraAddPoint.cpp
	src/Action/World/Camera/ActionCameraMoveSelection.cpp
	src/Action/World/Camera/ActionCameraMoveTimeSelection.cpp
	src/Action/World/Helper/ActionWorldTerrainEditHeight.cpp
	src/Action/World/Light/ActionWorldAddLight.cpp
	src/Action/World/Light/ActionWorldDeleteLight.cpp
	src/Action/World/Link/ActionWorldAddLink.cpp
	src/Action/World/Link/ActionWorldDeleteLink.cpp
	src/Action/World/Object/ActionWorldAddObject.cpp
	src/Action/World/Object/ActionWorldDeleteObject.cpp
	src/Action/World/Object/ActionWorldEditObject.cpp
	src/Action/World/Object/ActionWorldRotateObjects.cpp
	src/Action/World/Terrain/ActionWorldAddTerrain.cpp
	src/Action/World/Terrain/ActionWorldDeleteTerrain.cpp
	src/Action/World/Terrain/ActionWorldEditTerrain.cpp
	src/Action/World/Terrain/ActionWorldTerrainApplyHeightmap.cpp
	src/Action/World/Terrain/ActionWorldTerrainBrushExtrude.cpp
	src/Action/World/ActionWorldDeleteSelection.cpp
	src/Action/World/ActionWorldEditData.cpp
	src/Action/World/ActionWorldMoveSelection.cpp
	src/Action/World/ActionWorldPaste.cpp
	src/Action/World/ActionWorldSelectionAddComponent.cpp
	src/Action/World/ActionWorldSetEgo.cpp
	src/Action/Action.cpp
	src/Action/ActionGroup.cpp
	src/Action/ActionManager.cpp
	src/Action/ActionMultiView.cpp
	src/Data/Administration/AdminFile.cpp
	src/Data/Administration/AdminFileList.cpp
	src/Data/Administration/DataAdministration.cpp
	src/Data/Administration/GameIniData.cpp
	src/Data/Font/Import/ImporterCairo.cpp
	src/Data/Font/DataFont.cpp
	src/Data/Material/DataMaterial.cpp
	src/Data/Material/ShaderBuilderContext.cpp
	src/Data/Material/ShaderGraph.cpp
	src/Data/Material/ShaderNode.cpp
	src/Data/Model/Geometry/Geometry.cpp
	src/Data/Model/Geometry/GeometryBall.cpp
	src/Data/Model/Geometry/GeometryCube.cpp
	src/Data/Model/Geometry/GeometryCylinder.cpp
	src/Data/Model/Geometry/GeometryPlane.cpp
	src/Data/Model/Geometry/GeometryPlatonic.cpp
	src/Data/Model/Geometry/GeometrySphere.cpp
	src/Data/Model/Geometry/GeometrySubtract.cpp
	src/Data/Model/Geometry/GeometryTeapot.cpp
	src/Data/Model/Geometry/GeometryTorus.cpp
	src/Data/Model/Geometry/GeometryTorusKnot.cpp
	src/Data/Model/BspTree.cpp
	src/Data/Model/DataModel.cpp
	src/Data/Model/ModelMaterial.cpp
	src/Data/Model/ModelMesh.cpp
	src/Data/Model/ModelPolygon.cpp
	src/Data/Model/ModelSelection.cpp
	src/Data/Model/SkinGenerator.cpp
	src/Data/World/Lightmap/Lightmap.cpp
	src/Data/World/Lightmap/LightmapData.cpp
	src/Data/World/Lightmap/LightmapPhotonMap.cpp
	src/Data/World/Lightmap/LightmapPhotonMapImageSpace.cpp
	src/Data/World/Lightmap/LightmapRadiosity.cpp
	src/Data/World/Lightmap/LightmapRayTracing.cpp
	src/Data/World/DataCamera.cpp
	src/Data/World/DataWorld.cpp
	src/Data/World/WorldCamera.cpp
	src/Data/World/WorldLight.cpp
	src/Data/World/WorldLink.cpp
	src/Data/World/WorldObject.cpp
	src/Data/World/WorldTerrain.cpp
	src/Data/Data.cpp
	src/lib/any/any.cpp
	src/lib/base/array.cpp
	src/lib/base/pointer.cpp
	src/lib/base/strings.cpp
	src/lib/file/file.cpp
	src/lib/file/file_op.cpp
	src/lib/file/msg.cpp
	src/lib/file/path.cpp
	src/lib/hui/Controls/Control.cpp
	src/lib/hui/Controls/ControlButtonGtk.cpp
	src/lib/hui/Controls/ControlCheckBoxGtk.cpp
	src/lib/hui/Controls/ControlColorButtonGtk.cpp
	src/lib/hui/Controls/ControlComboBoxGtk.cpp
	src/lib/hui/Controls/ControlDrawingAreaGtk.cpp
	src/lib/hui/Controls/ControlEditGtk.cpp
	src/lib/hui/Controls/ControlExpanderGtk.cpp
	src/lib/hui/Controls/ControlGridGtk.cpp
	src/lib/hui/Controls/ControlGroupGtk.cpp
	src/lib/hui/Controls/ControlHeaderBar.cpp
	src/lib/hui/Controls/ControlLabelGtk.cpp
	src/lib/hui/Controls/ControlListViewGtk.cpp
	src/lib/hui/Controls/ControlMenuButtonGtk.cpp
	src/lib/hui/Controls/ControlMultilineEditGtk.cpp
	src/lib/hui/Controls/ControlPanedGtk.cpp
	src/lib/hui/Controls/ControlProgressBarGtk.cpp
	src/lib/hui/Controls/ControlRadioButtonGtk.cpp
	src/lib/hui/Controls/ControlScrollerGtk.cpp
	src/lib/hui/Controls/ControlSeparatorGtk.cpp
	src/lib/hui/Controls/ControlSliderGtk.cpp
	src/lib/hui/Controls/ControlSpinButtonGtk.cpp
	src/lib/hui/Controls/ControlTabControlGtk.cpp
	src/lib/hui/Controls/ControlToggleButtonGtk.cpp
	src/lib/hui/Controls/ControlTreeViewGtk.cpp
	src/lib/hui/Controls/MenuItemGtk.cpp
	src/lib/hui/Controls/MenuItemSeparatorGtk.cpp
	src/lib/hui/Controls/MenuItemSeparatorWin.cpp
	src/lib/hui/Controls/MenuItemSubmenuGtk.cpp
	src/lib/hui/Controls/MenuItemSubmenuWin.cpp
	src/lib/hui/Controls/MenuItemToggleGtk.cpp
	src/lib/hui/Controls/MenuItemToggleWin.cpp
	src/lib/hui/Controls/MenuItemWin.cpp
	src/lib/hui/Controls/ToolItemButtonGtk.cpp
	src/lib/hui/Controls/ToolItemButtonWin.cpp
	src/lib/hui/Controls/ToolItemMenuButtonGtk.cpp
	src/lib/hui/Controls/ToolItemMenuButtonWin.cpp
	src/lib/hui/Controls/ToolItemSeparatorGtk.cpp
	src/lib/hui/Controls/ToolItemSeparatorWin.cpp
	src/lib/hui/Controls/ToolItemToggleButtonGtk.cpp
	src/lib/hui/Controls/ToolItemToggleButtonWin.cpp
	src/lib/hui/Application.cpp
	src/lib/hui/Callback.cpp
	src/lib/hui/Clipboard.cpp
	src/lib/hui/common_dlg.cpp
	src/lib/hui/common_dlg_gtk.cpp
	src/lib/hui/common_dlg_win.cpp
	src/lib/hui/Config.cpp
	src/lib/hui/error.cpp
	src/lib/hui/Event.cpp
	src/lib/hui/hui.cpp
	src/lib/hui/language.cpp
	src/lib/hui/main_gtk.cpp
	src/lib/hui/main_win.cpp
	src/lib/hui/Menu.cpp
	src/lib/hui/MenuGtk.cpp
	src/lib/hui/MenuWin.cpp
	src/lib/hui/PainterGtk.cpp
	src/lib/hui/Panel.cpp
	src/lib/hui/Resource.cpp
	src/lib/hui/Timer.cpp
	src/lib/hui/Toolbar.cpp
	src/lib/hui/ToolbarGtk.cpp
	src/lib/hui/ToolbarWin.cpp
	src/lib/hui/utility.cpp
	src/lib/hui/Window.cpp
	src/lib/hui/window_control.cpp
	src/lib/hui/window_control_gtk.cpp
	src/lib/hui/window_control_win.cpp
	src/lib/hui/WindowGtk.cpp
	src/lib/hui/WindowWin.cpp
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
	src/lib/kaba/compiler/BackendX86.cpp
	src/lib/kaba/compiler/CommandList.cpp
	src/lib/kaba/compiler/compiler.cpp
	src/lib/kaba/compiler/mapper.cpp
	src/lib/kaba/compiler/serializer.cpp
	src/lib/kaba/compiler/SerializerX.cpp
	src/lib/kaba/compiler/SerialNode.cpp
	src/lib/kaba/dynamic/call.cpp
	src/lib/kaba/dynamic/dynamic.cpp
	src/lib/kaba/dynamic/exception.cpp
	src/lib/kaba/lib/dict.cpp
	src/lib/kaba/lib/extern.cpp
	src/lib/kaba/lib/lib.cpp
	src/lib/kaba/lib/lib_base.cpp
	src/lib/kaba/lib/lib_doc.cpp
	src/lib/kaba/lib/lib_hui.cpp
	src/lib/kaba/lib/lib_image.cpp
	src/lib/kaba/lib/lib_kaba.cpp
	src/lib/kaba/lib/lib_math.cpp
	src/lib/kaba/lib/lib_net.cpp
	src/lib/kaba/lib/lib_nix.cpp
	src/lib/kaba/lib/lib_os.cpp
	src/lib/kaba/lib/lib_threads.cpp
	src/lib/kaba/lib/lib_time.cpp
	src/lib/kaba/lib/lib_vulkan.cpp
	src/lib/kaba/syntax/Block.cpp
	src/lib/kaba/syntax/Class.cpp
	src/lib/kaba/syntax/Constant.cpp
	src/lib/kaba/syntax/Function.cpp
	src/lib/kaba/syntax/Identifier.cpp
	src/lib/kaba/syntax/implicit.cpp
	src/lib/kaba/syntax/import.cpp
	src/lib/kaba/syntax/lexical.cpp
	src/lib/kaba/syntax/macros.cpp
	src/lib/kaba/syntax/Node.cpp
	src/lib/kaba/syntax/Operator.cpp
	src/lib/kaba/syntax/Parser.cpp
	src/lib/kaba/syntax/preprocessor.cpp
	src/lib/kaba/syntax/Statement.cpp
	src/lib/kaba/syntax/SyntaxTree.cpp
	src/lib/kaba/syntax/Variable.cpp
	src/lib/kaba/CompilerConfiguration.cpp
	src/lib/kaba/Interpreter.cpp
	src/lib/kaba/kaba.cpp
	src/lib/math/complex.cpp
	src/lib/math/interpolation.cpp
	src/lib/math/math.cpp
	src/lib/math/matrix.cpp
	src/lib/math/matrix3.cpp
	src/lib/math/plane.cpp
	src/lib/math/quaternion.cpp
	src/lib/math/random.cpp
	src/lib/math/ray.cpp
	src/lib/math/rect.cpp
	src/lib/math/vec2.cpp
	src/lib/math/vec4.cpp
	src/lib/math/vector.cpp
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
	src/lib/threads/Mutex.cpp
	src/lib/threads/Thread.cpp
	src/lib/threads/ThreadedWork.cpp
	src/lib/xfile/chunked.cpp
	src/lib/xfile/pdf.cpp
	src/lib/xfile/xml.cpp
	src/Mode/Administration/Dialog/AdministrationDialog.cpp
	src/Mode/Administration/Dialog/ConfigurationDialog.cpp
	src/Mode/Administration/Dialog/NewProjectDialog.cpp
	src/Mode/Administration/ModeAdministration.cpp
	src/Mode/Font/Dialog/FontDialog.cpp
	src/Mode/Font/ModeFont.cpp
	src/Mode/Material/Dialog/MaterialPropertiesDialog.cpp
	src/Mode/Material/Dialog/ShaderGraphDialog.cpp
	src/Mode/Material/ModeMaterial.cpp
	src/Mode/Model/Animation/Creation/ModeModelAnimationInterpolateFrames.cpp
	src/Mode/Model/Animation/ModeModelAnimation.cpp
	src/Mode/Model/Animation/ModeModelAnimationNone.cpp
	src/Mode/Model/Animation/ModeModelAnimationSkeleton.cpp
	src/Mode/Model/Animation/ModeModelAnimationVertex.cpp
	src/Mode/Model/Dialog/ModelAnimationDialog.cpp
	src/Mode/Model/Dialog/ModelAnimationTimelinePanel.cpp
	src/Mode/Model/Dialog/ModelDuplicateAnimationDialog.cpp
	src/Mode/Model/Dialog/ModelEasifyDialog.cpp
	src/Mode/Model/Dialog/ModelFXDialog.cpp
	src/Mode/Model/Dialog/ModelMaterialDialog.cpp
	src/Mode/Model/Dialog/ModelMaterialSelectionDialog.cpp
	src/Mode/Model/Dialog/ModelNewAnimationDialog.cpp
	src/Mode/Model/Dialog/ModelPropertiesDialog.cpp
	src/Mode/Model/Mesh/Creation/ModeModelMeshAutoweld.cpp
	src/Mode/Model/Mesh/Creation/ModeModelMeshBevelEdges.cpp
	src/Mode/Model/Mesh/Creation/ModeModelMeshCreateBall.cpp
	src/Mode/Model/Mesh/Creation/ModeModelMeshCreateCube.cpp
	src/Mode/Model/Mesh/Creation/ModeModelMeshCreateCylinder.cpp
	src/Mode/Model/Mesh/Creation/ModeModelMeshCreateCylinderSnake.cpp
	src/Mode/Model/Mesh/Creation/ModeModelMeshCreatePlane.cpp
	src/Mode/Model/Mesh/Creation/ModeModelMeshCreatePlatonic.cpp
	src/Mode/Model/Mesh/Creation/ModeModelMeshCreatePolygon.cpp
	src/Mode/Model/Mesh/Creation/ModeModelMeshCreateTorus.cpp
	src/Mode/Model/Mesh/Creation/ModeModelMeshCreateVertex.cpp
	src/Mode/Model/Mesh/Creation/ModeModelMeshCutLoop.cpp
	src/Mode/Model/Mesh/Creation/ModeModelMeshDeformCylinder.cpp
	src/Mode/Model/Mesh/Creation/ModeModelMeshDeformFunction.cpp
	src/Mode/Model/Mesh/Creation/ModeModelMeshExtrudePolygons.cpp
	src/Mode/Model/Mesh/Creation/ModeModelMeshPaste.cpp
	src/Mode/Model/Mesh/Creation/ModeModelMeshSplitPolygon.cpp
	src/Mode/Model/Mesh/Selection/MeshSelectionMode.cpp
	src/Mode/Model/Mesh/Selection/MeshSelectionModeEdge.cpp
	src/Mode/Model/Mesh/Selection/MeshSelectionModePolygon.cpp
	src/Mode/Model/Mesh/Selection/MeshSelectionModeSurface.cpp
	src/Mode/Model/Mesh/Selection/MeshSelectionModeVertex.cpp
	src/Mode/Model/Mesh/ModeModelMesh.cpp
	src/Mode/Model/Mesh/ModeModelMeshDeform.cpp
	src/Mode/Model/Mesh/ModeModelMeshMaterial.cpp
	src/Mode/Model/Mesh/ModeModelMeshPaint.cpp
	src/Mode/Model/Mesh/ModeModelMeshTexture.cpp
	src/Mode/Model/Skeleton/Creation/ModeModelSkeletonAttachVertices.cpp
	src/Mode/Model/Skeleton/Creation/ModeModelSkeletonCreateBone.cpp
	src/Mode/Model/Skeleton/ModeModelSkeleton.cpp
	src/Mode/Model/ModeModel.cpp
	src/Mode/World/Camera/Creation/ModeWorldCameraCreatePoint.cpp
	src/Mode/World/Camera/ModeWorldCamera.cpp
	src/Mode/World/Creation/ModeWorldCreateLight.cpp
	src/Mode/World/Creation/ModeWorldCreateLink.cpp
	src/Mode/World/Creation/ModeWorldCreateObject.cpp
	src/Mode/World/Creation/ModeWorldCreateTerrain.cpp
	src/Mode/World/Dialog/CameraDialog.cpp
	src/Mode/World/Dialog/ComponentSelectionDialog.cpp
	src/Mode/World/Dialog/LightmapDialog.cpp
	src/Mode/World/Dialog/ObjectPropertiesDialog.cpp
	src/Mode/World/Dialog/ScriptVarsDialog.cpp
	src/Mode/World/Dialog/SelectionPropertiesDialog.cpp
	src/Mode/World/Dialog/TerrainCreationDialog.cpp
	src/Mode/World/Dialog/TerrainHeightmapDialog.cpp
	src/Mode/World/Dialog/TerrainPropertiesDialog.cpp
	src/Mode/World/Dialog/WorldObjectListPanel.cpp
	src/Mode/World/Dialog/WorldPropertiesDialog.cpp
	src/Mode/World/Terrain/ModeWorldTerrain.cpp
	src/Mode/World/ModeWorld.cpp
	src/Mode/Mode.cpp
	src/Mode/ModeCreation.cpp
	src/Mode/ModeNone.cpp
	src/MultiView/ActionController.cpp
	src/MultiView/CameraController.cpp
	src/MultiView/ColorScheme.cpp
	src/MultiView/DrawingHelper.cpp
	src/MultiView/MouseWrapper.cpp
	src/MultiView/MultiView.cpp
	src/MultiView/SingleData.cpp
	src/MultiView/Window.cpp
	src/Storage/Format/old/LoadModelV11.cpp
	src/Storage/Format/Format.cpp
	src/Storage/Format/FormatFontX.cpp
	src/Storage/Format/FormatMaterial.cpp
	src/Storage/Format/FormatModel.cpp
	src/Storage/Format/FormatModel3ds.cpp
	src/Storage/Format/FormatModelJson.cpp
	src/Storage/Format/FormatModelPly.cpp
	src/Storage/Format/FormatTerrain.cpp
	src/Storage/Format/FormatWorld.cpp
	src/Storage/Storage.cpp
	src/Stuff/BrushPanel.cpp
	src/Stuff/Observable.cpp
	src/Stuff/PluginManager.cpp
	src/Stuff/Progress.cpp
	src/y/components/Animator.cpp
	src/y/components/Collider.cpp
	src/y/components/Skeleton.cpp
	src/y/components/SolidBody.cpp
	src/y/Camera.cpp
	src/y/Component.cpp
	src/y/ComponentManager.cpp
	src/y/EngineData.cpp
	src/y/Entity.cpp
	src/y/Entity3D.cpp
	src/y/Font.cpp
	src/y/LevelData.cpp
	src/y/Link.cpp
	src/y/Material.cpp
	src/y/Model.cpp
	src/y/ModelManager.cpp
	src/y/Object.cpp
	src/y/ResourceManager.cpp
	src/y/Terrain.cpp
	src/y/World.cpp
	src/Edward.cpp
	src/main.cpp
	src/meta.cpp
)

find_package(PkgConfig REQUIRED)

# gtk3 or gtk4 mandatory
set(GTK3_OR_GTK4 "gtk3" CACHE STRING "Use gtk3 or gtk4?")
pkg_check_modules(GTK3 gtk+-3.0)
if(GTK3_FOUND)
	list(APPEND GTK3_OR_GTK4_FOUND "gtk3")
endif()
pkg_check_modules(GTK4 gtk4)
if(GTK4_FOUND)
	list(APPEND GTK3_OR_GTK4_FOUND "gtk4")
endif()
if(GTK3_OR_GTK4_FOUND)
	if(NOT ${GTK3_OR_GTK4} IN_LIST GTK3_OR_GTK4_FOUND)
		list(GET GTK3_OR_GTK4_FOUND 0 X)
		set(GTK3_OR_GTK4 ${X} CACHE STRING "choose" FORCE)
		message(WARNING "Choice for GTK3_OR_GTK4 invalid, using ${GTK3_OR_GTK4} instead")
	endif()
else()
	message(SEND_ERROR "Neither gtk3 or gtk4 found") #  or/nor..
	set(GTK3_OR_GTK4 "")
endif()
if(GTK3_OR_GTK4 STREQUAL "gtk3")
	include_directories(${GTK3_INCLUDE_DIRS})
	link_directories(${GTK3_LIBRARY_DIRS})
	list(APPEND DEPS ${GTK3_LIBRARIES})
	add_definitions(${GTK3_CFLAGS_OTHER})
	add_compile_definitions(HAS_LIB_GTK3=1)
endif()
if(GTK3_OR_GTK4 STREQUAL "gtk4")
	include_directories(${GTK4_INCLUDE_DIRS})
	link_directories(${GTK4_LIBRARY_DIRS})
	list(APPEND DEPS ${GTK4_LIBRARIES})
	add_definitions(${GTK4_CFLAGS_OTHER})
	add_compile_definitions(HAS_LIB_GTK4=1)
endif()

# zlib mandatory
pkg_check_modules(ZLIB REQUIRED zlib)
include_directories(${ZLIB_INCLUDE_DIRS})
link_directories(${ZLIB_LIBRARY_DIRS})
list(APPEND DEPS ${ZLIB_LIBRARIES})
add_definitions(${ZLIB_CFLAGS_OTHER})
add_compile_definitions(HAS_LIB_ZLIB=1)

# gl mandatory
pkg_check_modules(GL REQUIRED gl)
include_directories(${GL_INCLUDE_DIRS})
link_directories(${GL_LIBRARY_DIRS})
list(APPEND DEPS ${GL_LIBRARIES})
add_definitions(${GL_CFLAGS_OTHER})
add_compile_definitions(HAS_LIB_GL=1)

# unwind (optional)
pkg_check_modules(LIBUNWIND libunwind)
if(LIBUNWIND_FOUND)
	include_directories(${LIBUNWIND_INCLUDE_DIRS})
	link_directories(${LIBUNWIND_LIBRARY_DIRS})
	list(APPEND DEPS ${LIBUNWIND_LIBRARIES})
	add_definitions(${LIBUNWIND_CFLAGS_OTHER})
	add_compile_definitions(HAS_LIB_UNWIND=1)
else()
	add_compile_definitions(HAS_LIB_UNWIND=0)
endif()

# dl...
list(APPEND DEPS ${CMAKE_DL_LIBS})
add_compile_definitions(HAS_LIB_DL=1)

# xlib (optional)
pkg_check_modules(X11 x11)
if(X11_FOUND)
	include_directories(${X11_INCLUDE_DIRS})
	link_directories(${X11_LIBRARY_DIRS})
	list(APPEND DEPS ${X11_LIBRARIES})
	add_definitions(${X11_CFLAGS_OTHER})
	add_compile_definitions(HAS_LIB_XLIB=1)
else()
	add_compile_definitions(HAS_LIB_XLIB=0)
endif()

add_executable(edward ${SOURCE})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(edward PRIVATE Threads::Threads ${DEPS})

add_compile_definitions(INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}")

install(TARGETS edward DESTINATION bin)
install(DIRECTORY "static/icons" DESTINATION share/edward)
install(DIRECTORY "static/shader" DESTINATION share/edward)
install(FILES "static/edward.jpg" DESTINATION share/edward)
install(FILES "static/hui_resources.txt" DESTINATION share/edward)
install(FILES "static/icon.svg" DESTINATION share/edward)
install(FILES "static/keys.txt" DESTINATION share/edward)
install(FILES "static/license.txt" DESTINATION share/edward)
install(FILES "static/license_small.txt" DESTINATION share/edward)
install(DIRECTORY "Plugins" DESTINATION share/edward)

set(INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
configure_file("static/michisoft-edward.desktop" /tmp/michisoft-edward.desktop NO_SOURCE_PERMISSIONS)

find_program(XDG_DESKTOP_MENU_EXECUTABLE xdg-desktop-menu)
if(NOT XDG_DESKTOP_MENU_EXECUTABLE STREQUAL "XDG_DESKTOP_MENU_EXECUTABLE_NOTFOUND")
	install(CODE "message(STATUS \"installing desktop file\")")
	install(CODE "execute_process(COMMAND ${XDG_DESKTOP_MENU_EXECUTABLE} install /tmp/michisoft-edward.desktop)")
endif()

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
