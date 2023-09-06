# created by xmake

cmake_minimum_required(VERSION 3.21)
project(edward)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)
set(SOURCE
	src/action/font/ActionFontEditGlobal.cpp
	src/action/font/ActionFontEditGlyph.cpp
	src/action/material/ActionMaterialEditAppearance.cpp
	src/action/material/ActionMaterialEditPhysics.cpp
	src/action/model/animation/ActionModelAddAnimation.cpp
	src/action/model/animation/ActionModelAnimationAddFrame.cpp
	src/action/model/animation/ActionModelAnimationDeleteFrame.cpp
	src/action/model/animation/ActionModelAnimationSetBone.cpp
	src/action/model/animation/ActionModelAnimationSetData.cpp
	src/action/model/animation/ActionModelAnimationSetFrameDuration.cpp
	src/action/model/animation/ActionModelAnimationTransformBones.cpp
	src/action/model/animation/ActionModelAnimationTransformVertices.cpp
	src/action/model/animation/ActionModelDeleteAnimation.cpp
	src/action/model/animation/ActionModelDuplicateAnimation.cpp
	src/action/model/data/ActionModelAddMaterial.cpp
	src/action/model/data/ActionModelDeleteMaterial.cpp
	src/action/model/data/ActionModelEditData.cpp
	src/action/model/data/ActionModelEditMaterial.cpp
	src/action/model/mesh/brush/ActionModelBrushComplexify.cpp
	src/action/model/mesh/brush/ActionModelBrushExtrude.cpp
	src/action/model/mesh/brush/ActionModelBrushSmooth.cpp
	src/action/model/mesh/edge/ActionModelBevelEdges.cpp
	src/action/model/mesh/edge/ActionModelCollapseEdge.cpp
	src/action/model/mesh/edge/ActionModelSplitEdge.cpp
	src/action/model/mesh/effect/ActionModelAddEffects.cpp
	src/action/model/mesh/effect/ActionModelClearEffects.cpp
	src/action/model/mesh/effect/ActionModelEditEffect.cpp
	src/action/model/mesh/look/ActionModelSetMaterial.cpp
	src/action/model/mesh/look/ActionModelSetNormalModeSelection.cpp
	src/action/model/mesh/physical/ActionModelAddBall.cpp
	src/action/model/mesh/physical/ActionModelAddCylinder.cpp
	src/action/model/mesh/physical/ActionModelDeleteBall.cpp
	src/action/model/mesh/physical/ActionModelDeleteCylinder.cpp
	src/action/model/mesh/polygon/helper/ActionModelPolygonAddVertex.cpp
	src/action/model/mesh/polygon/helper/ActionModelPolygonRemoveVertex.cpp
	src/action/model/mesh/polygon/ActionModelAddPolygon.cpp
	src/action/model/mesh/polygon/ActionModelAddPolygonAutoSkin.cpp
	src/action/model/mesh/polygon/ActionModelAddPolygonSingleTexture.cpp
	src/action/model/mesh/polygon/ActionModelAddPolygonWithSkinGenerator.cpp
	src/action/model/mesh/polygon/ActionModelCutOutPolygons.cpp
	src/action/model/mesh/polygon/ActionModelExtrudePolygons.cpp
	src/action/model/mesh/polygon/ActionModelMergePolygonsSelection.cpp
	src/action/model/mesh/polygon/ActionModelSplitPolygon.cpp
	src/action/model/mesh/polygon/ActionModelTriangulateSelection.cpp
	src/action/model/mesh/skin/ActionModelAutomap.cpp
	src/action/model/mesh/skin/ActionModelSkinVerticesFromProjection.cpp
	src/action/model/mesh/skin/ActionModelTransformSkinVertices.cpp
	src/action/model/mesh/surface/helper/ActionModelAddEmptySurface.cpp
	src/action/model/mesh/surface/helper/ActionModelDeleteEmptySurface.cpp
	src/action/model/mesh/surface/helper/ActionModelJoinSurfaces.cpp
	src/action/model/mesh/surface/helper/ActionModelSurfaceAddPolygon.cpp
	src/action/model/mesh/surface/helper/ActionModelSurfaceDeletePolygon.cpp
	src/action/model/mesh/surface/helper/ActionModelSurfaceRelinkPolygon.cpp
	src/action/model/mesh/surface/ActionModelAutoWeldSelection.cpp
	src/action/model/mesh/surface/ActionModelDeleteSurface.cpp
	src/action/model/mesh/surface/ActionModelSurfaceAutoWeld.cpp
	src/action/model/mesh/surface/ActionModelSurfaceCopy.cpp
	src/action/model/mesh/surface/ActionModelSurfaceInvert.cpp
	src/action/model/mesh/surface/ActionModelSurfacesSubdivide.cpp
	src/action/model/mesh/surface/ActionModelSurfaceVolumeAnd.cpp
	src/action/model/mesh/surface/ActionModelSurfaceVolumeSubtract.cpp
	src/action/model/mesh/vertex/helper/ActionModelDeleteUnusedVertex.cpp
	src/action/model/mesh/vertex/helper/ActionModelMoveVertex.cpp
	src/action/model/mesh/vertex/ActionModelAddVertex.cpp
	src/action/model/mesh/vertex/ActionModelAlignToGrid.cpp
	src/action/model/mesh/vertex/ActionModelBevelVertices.cpp
	src/action/model/mesh/vertex/ActionModelCollapseVertices.cpp
	src/action/model/mesh/vertex/ActionModelDeleteVertex.cpp
	src/action/model/mesh/vertex/ActionModelFlattenVertices.cpp
	src/action/model/mesh/vertex/ActionModelNearifyVertices.cpp
	src/action/model/mesh/vertex/ActionModelTransformVertices.cpp
	src/action/model/mesh/vertex/ActionModelTriangulateVertices.cpp
	src/action/model/mesh/ActionModelDeleteSelection.cpp
	src/action/model/mesh/ActionModelEasify.cpp
	src/action/model/mesh/ActionModelPasteGeometry.cpp
	src/action/model/skeleton/ActionModelAddBone.cpp
	src/action/model/skeleton/ActionModelAttachVerticesToBone.cpp
	src/action/model/skeleton/ActionModelDeleteBone.cpp
	src/action/model/skeleton/ActionModelDeleteBoneSelection.cpp
	src/action/model/skeleton/ActionModelReconnectBone.cpp
	src/action/model/skeleton/ActionModelSetSubModel.cpp
	src/action/model/skeleton/ActionModelTransformBones.cpp
	src/action/world/camera/ActionCameraAddPoint.cpp
	src/action/world/camera/ActionCameraMoveSelection.cpp
	src/action/world/camera/ActionCameraMoveTimeSelection.cpp
	src/action/world/helper/ActionWorldTerrainEditHeight.cpp
	src/action/world/light/ActionWorldAddLight.cpp
	src/action/world/light/ActionWorldDeleteLight.cpp
	src/action/world/link/ActionWorldAddLink.cpp
	src/action/world/link/ActionWorldDeleteLink.cpp
	src/action/world/object/ActionWorldAddObject.cpp
	src/action/world/object/ActionWorldDeleteObject.cpp
	src/action/world/object/ActionWorldEditObject.cpp
	src/action/world/object/ActionWorldRotateObjects.cpp
	src/action/world/terrain/ActionWorldAddTerrain.cpp
	src/action/world/terrain/ActionWorldDeleteTerrain.cpp
	src/action/world/terrain/ActionWorldEditTerrain.cpp
	src/action/world/terrain/ActionWorldTerrainApplyHeightmap.cpp
	src/action/world/terrain/ActionWorldTerrainBrushExtrude.cpp
	src/action/world/ActionWorldDeleteSelection.cpp
	src/action/world/ActionWorldEditData.cpp
	src/action/world/ActionWorldMoveSelection.cpp
	src/action/world/ActionWorldPaste.cpp
	src/action/world/ActionWorldSelectionAddComponent.cpp
	src/action/world/ActionWorldSetEgo.cpp
	src/action/Action.cpp
	src/action/ActionGroup.cpp
	src/action/ActionManager.cpp
	src/action/ActionMultiView.cpp
	src/data/administration/AdminFile.cpp
	src/data/administration/AdminFileList.cpp
	src/data/administration/DataAdministration.cpp
	src/data/administration/GameIniData.cpp
	src/data/font/import/ImporterCairo.cpp
	src/data/font/DataFont.cpp
	src/data/material/DataMaterial.cpp
	src/data/material/ShaderBuilderContext.cpp
	src/data/material/ShaderGraph.cpp
	src/data/material/ShaderNode.cpp
	src/data/model/geometry/Geometry.cpp
	src/data/model/geometry/GeometryBall.cpp
	src/data/model/geometry/GeometryCube.cpp
	src/data/model/geometry/GeometryCylinder.cpp
	src/data/model/geometry/GeometryPlane.cpp
	src/data/model/geometry/GeometryPlatonic.cpp
	src/data/model/geometry/GeometrySphere.cpp
	src/data/model/geometry/GeometrySubtract.cpp
	src/data/model/geometry/GeometryTeapot.cpp
	src/data/model/geometry/GeometryTorus.cpp
	src/data/model/geometry/GeometryTorusKnot.cpp
	src/data/model/BspTree.cpp
	src/data/model/DataModel.cpp
	src/data/model/ModelMaterial.cpp
	src/data/model/ModelMesh.cpp
	src/data/model/ModelPolygon.cpp
	src/data/model/ModelSelection.cpp
	src/data/model/SkinGenerator.cpp
	src/data/world/lightmap/Lightmap.cpp
	src/data/world/lightmap/LightmapData.cpp
	src/data/world/lightmap/LightmapPhotonMap.cpp
	src/data/world/lightmap/LightmapPhotonMapImageSpace.cpp
	src/data/world/lightmap/LightmapRadiosity.cpp
	src/data/world/lightmap/LightmapRayTracing.cpp
	src/data/world/DataCamera.cpp
	src/data/world/DataWorld.cpp
	src/data/world/WorldCamera.cpp
	src/data/world/WorldLight.cpp
	src/data/world/WorldLink.cpp
	src/data/world/WorldObject.cpp
	src/data/world/WorldTerrain.cpp
	src/data/Data.cpp
	src/lib/any/any.cpp
	src/lib/base/array.cpp
	src/lib/base/pointer.cpp
	src/lib/base/strings.cpp
	src/lib/doc/chunked.cpp
	src/lib/doc/pdf.cpp
	src/lib/doc/ttf.cpp
	src/lib/doc/xml.cpp
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
	src/mode/administration/dialog/AdministrationDialog.cpp
	src/mode/administration/dialog/ConfigurationDialog.cpp
	src/mode/administration/dialog/NewProjectDialog.cpp
	src/mode/administration/ModeAdministration.cpp
	src/mode/font/dialog/FontDialog.cpp
	src/mode/font/ModeFont.cpp
	src/mode/material/dialog/MaterialPropertiesDialog.cpp
	src/mode/material/dialog/ShaderGraphDialog.cpp
	src/mode/material/ModeMaterial.cpp
	src/mode/model/animation/creation/ModeModelAnimationInterpolateFrames.cpp
	src/mode/model/animation/ModeModelAnimation.cpp
	src/mode/model/animation/ModeModelAnimationNone.cpp
	src/mode/model/animation/ModeModelAnimationSkeleton.cpp
	src/mode/model/animation/ModeModelAnimationVertex.cpp
	src/mode/model/dialog/ModelAnimationDialog.cpp
	src/mode/model/dialog/ModelAnimationTimelinePanel.cpp
	src/mode/model/dialog/ModelDuplicateAnimationDialog.cpp
	src/mode/model/dialog/ModelEasifyDialog.cpp
	src/mode/model/dialog/ModelFXDialog.cpp
	src/mode/model/dialog/ModelMaterialDialog.cpp
	src/mode/model/dialog/ModelMaterialSelectionDialog.cpp
	src/mode/model/dialog/ModelNewAnimationDialog.cpp
	src/mode/model/dialog/ModelPropertiesDialog.cpp
	src/mode/model/mesh/creation/ModeModelMeshAutoweld.cpp
	src/mode/model/mesh/creation/ModeModelMeshBevelEdges.cpp
	src/mode/model/mesh/creation/ModeModelMeshCreateBall.cpp
	src/mode/model/mesh/creation/ModeModelMeshCreateCube.cpp
	src/mode/model/mesh/creation/ModeModelMeshCreateCylinder.cpp
	src/mode/model/mesh/creation/ModeModelMeshCreateCylinderSnake.cpp
	src/mode/model/mesh/creation/ModeModelMeshCreatePlane.cpp
	src/mode/model/mesh/creation/ModeModelMeshCreatePlatonic.cpp
	src/mode/model/mesh/creation/ModeModelMeshCreatePolygon.cpp
	src/mode/model/mesh/creation/ModeModelMeshCreateTorus.cpp
	src/mode/model/mesh/creation/ModeModelMeshCreateVertex.cpp
	src/mode/model/mesh/creation/ModeModelMeshCutLoop.cpp
	src/mode/model/mesh/creation/ModeModelMeshDeformCylinder.cpp
	src/mode/model/mesh/creation/ModeModelMeshDeformFunction.cpp
	src/mode/model/mesh/creation/ModeModelMeshExtrudePolygons.cpp
	src/mode/model/mesh/creation/ModeModelMeshPaste.cpp
	src/mode/model/mesh/creation/ModeModelMeshSplitPolygon.cpp
	src/mode/model/mesh/selection/MeshSelectionMode.cpp
	src/mode/model/mesh/selection/MeshSelectionModeEdge.cpp
	src/mode/model/mesh/selection/MeshSelectionModePolygon.cpp
	src/mode/model/mesh/selection/MeshSelectionModeSurface.cpp
	src/mode/model/mesh/selection/MeshSelectionModeVertex.cpp
	src/mode/model/mesh/ModeModelMesh.cpp
	src/mode/model/mesh/ModeModelMeshDeform.cpp
	src/mode/model/mesh/ModeModelMeshMaterial.cpp
	src/mode/model/mesh/ModeModelMeshPaint.cpp
	src/mode/model/mesh/ModeModelMeshTexture.cpp
	src/mode/model/skeleton/creation/ModeModelSkeletonAttachVertices.cpp
	src/mode/model/skeleton/creation/ModeModelSkeletonCreateBone.cpp
	src/mode/model/skeleton/ModeModelSkeleton.cpp
	src/mode/model/ModeModel.cpp
	src/mode/world/camera/creation/ModeWorldCameraCreatePoint.cpp
	src/mode/world/camera/ModeWorldCamera.cpp
	src/mode/world/creation/ModeWorldCreateLight.cpp
	src/mode/world/creation/ModeWorldCreateLink.cpp
	src/mode/world/creation/ModeWorldCreateObject.cpp
	src/mode/world/creation/ModeWorldCreateTerrain.cpp
	src/mode/world/dialog/CameraDialog.cpp
	src/mode/world/dialog/ComponentSelectionDialog.cpp
	src/mode/world/dialog/LightmapDialog.cpp
	src/mode/world/dialog/ObjectPropertiesDialog.cpp
	src/mode/world/dialog/ScriptVarsDialog.cpp
	src/mode/world/dialog/SelectionPropertiesDialog.cpp
	src/mode/world/dialog/TerrainCreationDialog.cpp
	src/mode/world/dialog/TerrainHeightmapDialog.cpp
	src/mode/world/dialog/TerrainPropertiesDialog.cpp
	src/mode/world/dialog/WorldObjectListPanel.cpp
	src/mode/world/dialog/WorldPropertiesDialog.cpp
	src/mode/world/terrain/ModeWorldTerrain.cpp
	src/mode/world/ModeWorld.cpp
	src/mode/Mode.cpp
	src/mode/ModeCreation.cpp
	src/mode/ModeNone.cpp
	src/multiview/ActionController.cpp
	src/multiview/CameraController.cpp
	src/multiview/ColorScheme.cpp
	src/multiview/DrawingHelper.cpp
	src/multiview/MouseWrapper.cpp
	src/multiview/MultiView.cpp
	src/multiview/SingleData.cpp
	src/multiview/Window.cpp
	src/storage/format/old/LoadModelV11.cpp
	src/storage/format/Format.cpp
	src/storage/format/FormatFontX.cpp
	src/storage/format/FormatMaterial.cpp
	src/storage/format/FormatModel.cpp
	src/storage/format/FormatModel3ds.cpp
	src/storage/format/FormatModelJson.cpp
	src/storage/format/FormatModelPly.cpp
	src/storage/format/FormatTerrain.cpp
	src/storage/format/FormatWorld.cpp
	src/storage/Storage.cpp
	src/stuff/BrushPanel.cpp
	src/stuff/PluginManager.cpp
	src/stuff/Progress.cpp
	src/y/components/Animator.cpp
	src/y/components/Collider.cpp
	src/y/components/MultiInstance.cpp
	src/y/components/Skeleton.cpp
	src/y/components/SolidBody.cpp
	src/y/components/UserMesh.cpp
	src/y/BaseClass.cpp
	src/y/Camera.cpp
	src/y/Component.cpp
	src/y/ComponentManager.cpp
	src/y/EngineData.cpp
	src/y/Entity.cpp
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

add_executable(edward ${SOURCE})

find_package(PkgConfig REQUIRED)


# lib: gtk3 or gtk4 (mandatory)
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

# lib: zlib mandatory
pkg_check_modules(ZLIB REQUIRED zlib)
include_directories(${ZLIB_INCLUDE_DIRS})
link_directories(${ZLIB_LIBRARY_DIRS})
list(APPEND DEPS ${ZLIB_LIBRARIES})
add_definitions(${ZLIB_CFLAGS_OTHER})
add_compile_definitions(HAS_LIB_ZLIB=1)

# lib: gl mandatory
pkg_check_modules(GL REQUIRED gl)
include_directories(${GL_INCLUDE_DIRS})
link_directories(${GL_LIBRARY_DIRS})
list(APPEND DEPS ${GL_LIBRARIES})
add_definitions(${GL_CFLAGS_OTHER})
add_compile_definitions(HAS_LIB_GL=1)

# lib: unwind (optional)
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

# lib: dl...
list(APPEND DEPS ${CMAKE_DL_LIBS})
add_compile_definitions(HAS_LIB_DL=1)

# lib: xlib (optional)
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
install(DIRECTORY "plugins" DESTINATION share/edward)

# install .desktop file
set(INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
configure_file("static/michisoft-edward.desktop" /tmp/michisoft-edward.desktop NO_SOURCE_PERMISSIONS)
install(FILES /tmp/michisoft-edward.desktop DESTINATION share/applications)

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
