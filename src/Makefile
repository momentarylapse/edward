# Project: Edward

BIN = ../edward
all : $(BIN)
OBJ = Action/Action.o \
 Action/ActionGroup.o \
 Action/ActionManager.o \
 Action/ActionMultiView.o \
 Action/Font/ActionFontEditGlobal.o \
 Action/Font/ActionFontEditGlyph.o \
 Action/Material/ActionMaterialEditAppearance.o \
 Action/Material/ActionMaterialEditPhysics.o \
 Action/Model/Animation/ActionModelAddAnimation.o \
 Action/Model/Animation/ActionModelAnimationAddFrame.o \
 Action/Model/Animation/ActionModelAnimationDeleteFrame.o \
 Action/Model/Animation/ActionModelAnimationSetBone.o \
 Action/Model/Animation/ActionModelAnimationSetData.o \
 Action/Model/Animation/ActionModelAnimationSetFrameDuration.o \
 Action/Model/Animation/ActionModelAnimationTransformBones.o \
 Action/Model/Animation/ActionModelAnimationTransformVertices.o \
 Action/Model/Animation/ActionModelDeleteAnimation.o \
 Action/Model/Animation/ActionModelDuplicateAnimation.o \
 Action/Model/Data/ActionModelAddMaterial.o \
 Action/Model/Data/ActionModelEditData.o \
 Action/Model/Data/ActionModelEditMaterial.o \
 Action/Model/Mesh/ActionModelDeleteSelection.o \
 Action/Model/Mesh/ActionModelEasify.o \
 Action/Model/Mesh/ActionModelPasteGeometry.o \
 Action/Model/Mesh/Brush/ActionModelBrushComplexify.o \
 Action/Model/Mesh/Brush/ActionModelBrushExtrude.o \
 Action/Model/Mesh/Brush/ActionModelBrushSmooth.o \
 Action/Model/Mesh/Edge/ActionModelBevelEdges.o \
 Action/Model/Mesh/Edge/ActionModelCollapseEdge.o \
 Action/Model/Mesh/Edge/ActionModelSplitEdge.o \
 Action/Model/Mesh/Effects/ActionModelAddEffects.o \
 Action/Model/Mesh/Effects/ActionModelClearEffects.o \
 Action/Model/Mesh/Effects/ActionModelEditEffect.o \
 Action/Model/Mesh/Look/ActionModelSetMaterial.o \
 Action/Model/Mesh/Look/ActionModelSetNormalModeSelection.o \
 Action/Model/Mesh/Polygon/ActionModelAddPolygonAutoSkin.o \
 Action/Model/Mesh/Polygon/ActionModelAddPolygon.o \
 Action/Model/Mesh/Polygon/ActionModelAddPolygonSingleTexture.o \
 Action/Model/Mesh/Polygon/ActionModelAddPolygonWithSkinGenerator.o \
 Action/Model/Mesh/Polygon/ActionModelCutOutPolygons.o \
 Action/Model/Mesh/Polygon/ActionModelExtrudePolygons.o \
 Action/Model/Mesh/Polygon/ActionModelMergePolygonsSelection.o \
 Action/Model/Mesh/Polygon/ActionModelSplitPolygon.o \
 Action/Model/Mesh/Polygon/ActionModelTriangulateSelection.o \
 Action/Model/Mesh/Polygon/Helper/ActionModelPolygonAddVertex.o \
 Action/Model/Mesh/Polygon/Helper/ActionModelPolygonRemoveVertex.o \
 Action/Model/Mesh/Skin/ActionModelAutomap.o \
 Action/Model/Mesh/Skin/ActionModelSkinVerticesFromProjection.o \
 Action/Model/Mesh/Skin/ActionModelTransformSkinVertices.o \
 Action/Model/Mesh/Surface/ActionModelAutoWeldSelection.o \
 Action/Model/Mesh/Surface/ActionModelDeleteSurface.o \
 Action/Model/Mesh/Surface/ActionModelSurfaceAutoWeld.o \
 Action/Model/Mesh/Surface/ActionModelSurfaceCopy.o \
 Action/Model/Mesh/Surface/ActionModelSurfaceInvert.o \
 Action/Model/Mesh/Surface/ActionModelSurfacesSubdivide.o \
 Action/Model/Mesh/Surface/ActionModelSurfaceVolumeAnd.o \
 Action/Model/Mesh/Surface/ActionModelSurfaceVolumeSubtract.o \
 Action/Model/Mesh/Surface/Helper/ActionModelAddEmptySurface.o \
 Action/Model/Mesh/Surface/Helper/ActionModelDeleteEmptySurface.o \
 Action/Model/Mesh/Surface/Helper/ActionModelJoinSurfaces.o \
 Action/Model/Mesh/Surface/Helper/ActionModelSurfaceAddPolygon.o \
 Action/Model/Mesh/Surface/Helper/ActionModelSurfaceDeletePolygon.o \
 Action/Model/Mesh/Surface/Helper/ActionModelSurfaceRelinkPolygon.o \
 Action/Model/Mesh/Vertex/ActionModelAddVertex.o \
 Action/Model/Mesh/Vertex/ActionModelAlignToGrid.o \
 Action/Model/Mesh/Vertex/ActionModelBevelVertices.o \
 Action/Model/Mesh/Vertex/ActionModelCollapseVertices.o \
 Action/Model/Mesh/Vertex/ActionModelDeleteVertex.o \
 Action/Model/Mesh/Vertex/ActionModelFlattenVertices.o \
 Action/Model/Mesh/Vertex/ActionModelNearifyVertices.o \
 Action/Model/Mesh/Vertex/ActionModelTransformVertices.o \
 Action/Model/Mesh/Vertex/ActionModelTriangulateVertices.o \
 Action/Model/Mesh/Vertex/Helper/ActionModelDeleteUnusedVertex.o \
 Action/Model/Mesh/Vertex/Helper/ActionModelMoveVertex.o \
 Action/Model/Skeleton/ActionModelAddBone.o \
 Action/Model/Skeleton/ActionModelAttachVerticesToBone.o \
 Action/Model/Skeleton/ActionModelDeleteBone.o \
 Action/Model/Skeleton/ActionModelDeleteBoneSelection.o \
 Action/Model/Skeleton/ActionModelReconnectBone.o \
 Action/Model/Skeleton/ActionModelSetSubModel.o \
 Action/Model/Skeleton/ActionModelTransformBones.o \
 Action/World/ActionWorldDeleteSelection.o \
 Action/World/ActionWorldEditData.o \
 Action/World/ActionWorldMoveSelection.o \
 Action/World/ActionWorldPaste.o \
 Action/World/ActionWorldSetEgo.o \
 Action/World/Camera/ActionCameraAddPoint.o \
 Action/World/Camera/ActionCameraMoveSelection.o \
 Action/World/Camera/ActionCameraMoveTimeSelection.o \
 Action/World/Helper/ActionWorldTerrainEditHeight.o \
 Action/World/Object/ActionWorldAddObject.o \
 Action/World/Object/ActionWorldDeleteObject.o \
 Action/World/Object/ActionWorldEditObject.o \
 Action/World/Object/ActionWorldRotateObjects.o \
 Action/World/Terrain/ActionWorldAddTerrain.o \
 Action/World/Terrain/ActionWorldDeleteTerrain.o \
 Action/World/Terrain/ActionWorldEditTerrain.o \
 Action/World/Terrain/ActionWorldTerrainApplyHeightmap.o \
 Action/World/Terrain/ActionWorldTerrainBrushExtrude.o \
 Data/Administration/AdminFile.o \
 Data/Administration/AdminFileList.o \
 Data/Administration/DataAdministration.o \
 Data/Administration/GameIniData.o \
 Data/Data.o \
 Data/Font/DataFont.o \
 Data/Font/Import/ImporterCairo.o \
 Data/Material/DataMaterial.o \
 Data/Model/BspTree.o \
 Data/Model/DataModel.o \
 Data/Model/Export/ExporterJson.o \
 Data/Model/Geometry/GeometryBall.o \
 Data/Model/Geometry/Geometry.o \
 Data/Model/Geometry/GeometryCube.o \
 Data/Model/Geometry/GeometryCylinder.o \
 Data/Model/Geometry/GeometryPlane.o \
 Data/Model/Geometry/GeometryPlatonic.o \
 Data/Model/Geometry/GeometrySphere.o \
 Data/Model/Geometry/GeometrySubtract.o \
 Data/Model/Geometry/GeometryTeapot.o \
 Data/Model/Geometry/GeometryTorus.o \
 Data/Model/Geometry/GeometryTorusKnot.o \
 Data/Model/Import/Importer3ds.o \
 Data/Model/Import/ImporterJson.o \
 Data/Model/Import/ImporterPly.o \
 Data/Model/ModelMaterial.o \
 Data/Model/ModelPolygon.o \
 Data/Model/ModelSurface.o \
 Data/Model/SkinGenerator.o \
 Data/World/DataCamera.o \
 Data/World/DataWorld.o \
 Data/World/Lightmap/Lightmap.o \
 Data/World/Lightmap/LightmapData.o \
 Data/World/Lightmap/LightmapPhotonMap.o \
 Data/World/Lightmap/LightmapPhotonMapImageSpace.o \
 Data/World/Lightmap/LightmapRadiosity.o \
 Data/World/Lightmap/LightmapRayTracing.o \
 Edward.o \
 lib/base/array.o \
 lib/base/strings.o \
 lib/file/file.o \
 lib/file/file_op.o \
 lib/file/msg.o \
 lib/hui/Application.o \
 lib/hui/Callback.o \
 lib/hui/Clipboard.o \
 lib/hui/common_dlg.o \
 lib/hui/common_dlg_gtk.o \
 lib/hui/common_dlg_win.o \
 lib/hui/Config.o \
 lib/hui/Controls/ControlButtonGtk.o \
 lib/hui/Controls/ControlCheckBoxGtk.o \
 lib/hui/Controls/ControlColorButtonGtk.o \
 lib/hui/Controls/ControlComboBoxGtk.o \
 lib/hui/Controls/Control.o \
 lib/hui/Controls/ControlDrawingAreaGtk.o \
 lib/hui/Controls/ControlEditGtk.o \
 lib/hui/Controls/ControlExpanderGtk.o \
 lib/hui/Controls/ControlGridGtk.o \
 lib/hui/Controls/ControlGroupGtk.o \
 lib/hui/Controls/ControlLabelGtk.o \
 lib/hui/Controls/ControlListViewGtk.o \
 lib/hui/Controls/ControlMultilineEditGtk.o \
 lib/hui/Controls/ControlPanedGtk.o \
 lib/hui/Controls/ControlProgressBarGtk.o \
 lib/hui/Controls/ControlRadioButtonGtk.o \
 lib/hui/Controls/ControlRevealerGtk.o \
 lib/hui/Controls/ControlScrollerGtk.o \
 lib/hui/Controls/ControlSeparatorGtk.o \
 lib/hui/Controls/ControlSliderGtk.o \
 lib/hui/Controls/ControlSpinButtonGtk.o \
 lib/hui/Controls/ControlTabControlGtk.o \
 lib/hui/Controls/ControlToggleButtonGtk.o \
 lib/hui/Controls/ControlTreeViewGtk.o \
 lib/hui/Controls/MenuItemGtk.o \
 lib/hui/Controls/MenuItemSeparatorGtk.o \
 lib/hui/Controls/MenuItemSeparatorWin.o \
 lib/hui/Controls/MenuItemSubmenuGtk.o \
 lib/hui/Controls/MenuItemSubmenuWin.o \
 lib/hui/Controls/MenuItemToggleGtk.o \
 lib/hui/Controls/MenuItemToggleWin.o \
 lib/hui/Controls/MenuItemWin.o \
 lib/hui/Controls/ToolItemButtonGtk.o \
 lib/hui/Controls/ToolItemButtonWin.o \
 lib/hui/Controls/ToolItemMenuButtonGtk.o \
 lib/hui/Controls/ToolItemMenuButtonWin.o \
 lib/hui/Controls/ToolItemSeparatorGtk.o \
 lib/hui/Controls/ToolItemSeparatorWin.o \
 lib/hui/Controls/ToolItemToggleButtonGtk.o \
 lib/hui/Controls/ToolItemToggleButtonWin.o \
 lib/hui/error.o \
 lib/hui/Event.o \
 lib/hui/hui.o \
 lib/hui/language.o \
 lib/hui/main_gtk.o \
 lib/hui/main_win.o \
 lib/hui/Menu.o \
 lib/hui/MenuGtk.o \
 lib/hui/MenuWin.o \
 lib/hui/PainterGtk.o \
 lib/hui/Panel.o \
 lib/hui/Resource.o \
 lib/hui/Timer.o \
 lib/hui/Toolbar.o \
 lib/hui/ToolbarGtk.o \
 lib/hui/ToolbarWin.o \
 lib/hui/utility.o \
 lib/hui/window_control.o \
 lib/hui/window_control_gtk.o \
 lib/hui/window_control_win.o \
 lib/hui/Window.o \
 lib/hui/WindowGtk.o \
 lib/hui/WindowWin.o \
 lib/image/color.o \
 lib/image/image_bmp.o \
 lib/image/image.o \
 lib/image/image_jpg.o \
 lib/image/ImagePainter.o \
 lib/image/image_png.o \
 lib/image/image_tga.o \
 lib/kaba/asm/asm.o \
 lib/kaba/compiler/compiler.o \
 lib/kaba/compiler/mapper.o \
 lib/kaba/compiler/serializer_amd64.o \
 lib/kaba/compiler/serializer_arm.o \
 lib/kaba/compiler/serializer.o \
 lib/kaba/compiler/serializer_x86.o \
 lib/kaba/kaba.o \
 lib/kaba/lib/exception.o \
 lib/kaba/lib/lib.o \
 lib/kaba/lib/lib_file.o \
 lib/kaba/lib/lib_hui.o \
 lib/kaba/lib/lib_image.o \
 lib/kaba/lib/lib_math.o \
 lib/kaba/lib/lib_net.o \
 lib/kaba/lib/lib_nix.o \
 lib/kaba/lib/lib_sound.o \
 lib/kaba/lib/lib_threads.o \
 lib/kaba/lib/lib_x.o \
 lib/kaba/syntax/class.o \
 lib/kaba/syntax/implicit.o \
 lib/kaba/syntax/lexical.o \
 lib/kaba/syntax/parser.o \
 lib/kaba/syntax/precompiler.o \
 lib/kaba/syntax/preprocessor.o \
 lib/kaba/syntax/syntax_tree.o \
 lib/math/complex.o \
 lib/math/interpolation.o \
 lib/math/math.o \
 lib/math/matrix3.o \
 lib/math/matrix.o \
 lib/math/plane.o \
 lib/math/quaternion.o \
 lib/math/random.o \
 lib/math/ray.o \
 lib/math/rect.o \
 lib/math/vector.o \
 lib/net/net.o \
 lib/nix/nix.o \
 lib/nix/nix_draw.o \
 lib/nix/nix_light.o \
 lib/nix/nix_shader.o \
 lib/nix/nix_textures.o \
 lib/nix/nix_vertexbuffer.o \
 lib/nix/nix_view.o \
 lib/threads/Mutex.o \
 lib/threads/Thread.o \
 lib/threads/ThreadedWork.o \
 meta.o \
 Mode/Administration/Dialog/AdministrationDialog.o \
 Mode/Administration/Dialog/ConfigurationDialog.o \
 Mode/Administration/ModeAdministration.o \
 Mode/Font/Dialog/FontDialog.o \
 Mode/Font/ModeFont.o \
 Mode/Material/Dialog/MaterialPropertiesDialog.o \
 Mode/Material/ModeMaterial.o \
 Mode/Mode.o \
 Mode/ModeCreation.o \
 Mode/Model/Animation/Creation/ModeModelAnimationInterpolateFrames.o \
 Mode/Model/Animation/ModeModelAnimation.o \
 Mode/Model/Animation/ModeModelAnimationNone.o \
 Mode/Model/Animation/ModeModelAnimationSkeleton.o \
 Mode/Model/Animation/ModeModelAnimationVertex.o \
 Mode/Model/Dialog/ModelAnimationDialog.o \
 Mode/Model/Dialog/ModelAnimationTimelinePanel.o \
 Mode/Model/Dialog/ModelDuplicateAnimationDialog.o \
 Mode/Model/Dialog/ModelEasifyDialog.o \
 Mode/Model/Dialog/ModelFXDialog.o \
 Mode/Model/Dialog/ModelMaterialDialog.o \
 Mode/Model/Dialog/ModelMaterialSelectionDialog.o \
 Mode/Model/Dialog/ModelNewAnimationDialog.o \
 Mode/Model/Dialog/ModelPropertiesDialog.o \
 Mode/Model/Mesh/Creation/ModeModelMeshAutoweld.o \
 Mode/Model/Mesh/Creation/ModeModelMeshBevelEdges.o \
 Mode/Model/Mesh/Creation/ModeModelMeshBrush.o \
 Mode/Model/Mesh/Creation/ModeModelMeshCreateBall.o \
 Mode/Model/Mesh/Creation/ModeModelMeshCreateCube.o \
 Mode/Model/Mesh/Creation/ModeModelMeshCreateCylinder.o \
 Mode/Model/Mesh/Creation/ModeModelMeshCreateCylinderSnake.o \
 Mode/Model/Mesh/Creation/ModeModelMeshCreatePlane.o \
 Mode/Model/Mesh/Creation/ModeModelMeshCreatePlatonic.o \
 Mode/Model/Mesh/Creation/ModeModelMeshCreatePolygon.o \
 Mode/Model/Mesh/Creation/ModeModelMeshCreateTorus.o \
 Mode/Model/Mesh/Creation/ModeModelMeshCreateVertex.o \
 Mode/Model/Mesh/Creation/ModeModelMeshDeformCylinder.o \
 Mode/Model/Mesh/Creation/ModeModelMeshDeformFunction.o \
 Mode/Model/Mesh/Creation/ModeModelMeshExtrudePolygons.o \
 Mode/Model/Mesh/Creation/ModeModelMeshPaste.o \
 Mode/Model/Mesh/Creation/ModeModelMeshSplitPolygon.o \
 Mode/Model/Mesh/ModeModelMesh.o \
 Mode/Model/Mesh/ModeModelMeshTexture.o \
 Mode/Model/Mesh/Selection/MeshSelectionMode.o \
 Mode/Model/Mesh/Selection/MeshSelectionModeEdge.o \
 Mode/Model/Mesh/Selection/MeshSelectionModePolygon.o \
 Mode/Model/Mesh/Selection/MeshSelectionModeSurface.o \
 Mode/Model/Mesh/Selection/MeshSelectionModeVertex.o \
 Mode/Model/ModeModel.o \
 Mode/Model/Skeleton/Creation/ModeModelSkeletonAttachVertices.o \
 Mode/Model/Skeleton/Creation/ModeModelSkeletonCreateBone.o \
 Mode/Model/Skeleton/ModeModelSkeleton.o \
 Mode/ModeNone.o \
 Mode/Welcome/ModeWelcome.o \
 Mode/World/Camera/Creation/ModeWorldCameraCreatePoint.o \
 Mode/World/Camera/ModeWorldCamera.o \
 Mode/World/Creation/ModeWorldCreateObject.o \
 Mode/World/Creation/ModeWorldCreateTerrain.o \
 Mode/World/Dialog/CameraDialog.o \
 Mode/World/Dialog/LightmapDialog.o \
 Mode/World/Dialog/ObjectPropertiesDialog.o \
 Mode/World/Dialog/SelectionPropertiesDialog.o \
 Mode/World/Dialog/TerrainCreationDialog.o \
 Mode/World/Dialog/TerrainHeightmapDialog.o \
 Mode/World/Dialog/TerrainPropertiesDialog.o \
 Mode/World/Dialog/WorldPropertiesDialog.o \
 Mode/World/ModeWorld.o \
 Mode/World/Terrain/ModeWorldEditTerrain.o \
 MultiView/ActionController.o \
 MultiView/CameraController.o \
 MultiView/MultiView.o \
 MultiView/SingleData.o \
 MultiView/Window.o \
 Stuff/Observable.o \
 Stuff/Observer.o \
 Stuff/PluginManager.o \
 Stuff/Progress.o \
 x/camera.o \
 x/font.o \
 x/material.o \
 x/model.o \
 x/model_manager.o \
 x/object.o \
 x/terrain.o \
 x/world.o

CPP = g++
GLOBALFLAGS = -O2 -g
#GLOBALFLAGS = -O1 -g
LIB_DIR = lib
CXXFLAGS = -std=c++11 `pkg-config --cflags gtk+-3.0` -DUSE_UNWIND=1 $(GLOBALFLAGS)
CXXFLAGSLINKING = -std=c++11 `pkg-config --cflags --libs gtk+-3.0` -lX11 -lz -lGL -lunwind $(GLOBALFLAGS)

$(BIN) : $(OBJ) main.o
	$(CPP) $(OBJ) main.o -o $(BIN) $(CXXFLAGSLINKING)




#all : $(BIN)

install : all
	mkdir -p /usr/share/edward
	cp -rf ../static/*           /usr/share/edward
	cp -rf ../Plugins           /usr/share/edward
	xdg-desktop-menu install ../static/michisoft-edward.desktop
	xdg-mime install ../static/michisoft-x-model.xml
	xdg-icon-resource install --context mimetypes --size  32 ../static/icons/png/model_32.png  application-x-michisoft-x-model --noupdate
	xdg-icon-resource install --context mimetypes --size  48 ../static/icons/png/model_48.png  application-x-michisoft-x-model --noupdate
	xdg-icon-resource install --context mimetypes --size  64 ../static/icons/png/model_64.png  application-x-michisoft-x-model --noupdate
	xdg-icon-resource install --context mimetypes --size  128 ../static/icons/png/model_128.png  application-x-michisoft-x-model --noupdate
	xdg-mime install ../static/michisoft-x-font.xml
	xdg-icon-resource install --context mimetypes --size  32 ../static/icons/png/font_32.png  application-x-michisoft-x-font --noupdate
	xdg-icon-resource install --context mimetypes --size  48 ../static/icons/png/font_48.png  application-x-michisoft-x-font --noupdate
	xdg-icon-resource install --context mimetypes --size  64 ../static/icons/png/font_64.png  application-x-michisoft-x-font --noupdate
	xdg-icon-resource install --context mimetypes --size  128 ../static/icons/png/font_128.png  application-x-michisoft-x-font --noupdate
	xdg-mime install ../static/michisoft-x-material.xml
	xdg-icon-resource install --context mimetypes --size  32 ../static/icons/png/material_32.png  application-x-michisoft-x-material --noupdate
	xdg-icon-resource install --context mimetypes --size  48 ../static/icons/png/material_48.png  application-x-michisoft-x-material --noupdate
	xdg-icon-resource install --context mimetypes --size  64 ../static/icons/png/material_64.png  application-x-michisoft-x-material --noupdate
	xdg-icon-resource install --context mimetypes --size  128 ../static/icons/png/material_128.png  application-x-michisoft-x-material --noupdate
	xdg-mime install ../static/michisoft-x-world.xml
	xdg-icon-resource install --context mimetypes --size  32 ../static/icons/png/world_32.png  application-x-michisoft-x-world --noupdate
	xdg-icon-resource install --context mimetypes --size  48 ../static/icons/png/world_48.png  application-x-michisoft-x-world --noupdate
	xdg-icon-resource install --context mimetypes --size  64 ../static/icons/png/world_64.png  application-x-michisoft-x-world --noupdate
	xdg-icon-resource install --context mimetypes --size  128 ../static/icons/png/world_128.png  application-x-michisoft-x-world --noupdate
	xdg-mime install ../static/michisoft-x-terrain.xml
	xdg-icon-resource install --context mimetypes --size  32 ../static/icons/png/terrain_32.png  application-x-michisoft-x-terrain --noupdate
	xdg-icon-resource install --context mimetypes --size  48 ../static/icons/png/terrain_48.png  application-x-michisoft-x-terrain --noupdate
	xdg-icon-resource install --context mimetypes --size  64 ../static/icons/png/terrain_64.png  application-x-michisoft-x-terrain --noupdate
	xdg-icon-resource install --context mimetypes --size  128 ../static/icons/png/terrain_128.png  application-x-michisoft-x-terrain
	cp $(BIN) /usr/bin




Action/Action.o : Action/Action.cpp
	$(CPP) -c Action/Action.cpp -o $@ $(CXXFLAGS)

Action/ActionGroup.o : Action/ActionGroup.cpp
	$(CPP) -c Action/ActionGroup.cpp -o $@ $(CXXFLAGS)

Action/ActionManager.o : Action/ActionManager.cpp
	$(CPP) -c Action/ActionManager.cpp -o $@ $(CXXFLAGS)

Action/ActionMultiView.o : Action/ActionMultiView.cpp
	$(CPP) -c Action/ActionMultiView.cpp -o $@ $(CXXFLAGS)

Action/Font/ActionFontEditGlobal.o : Action/Font/ActionFontEditGlobal.cpp
	$(CPP) -c Action/Font/ActionFontEditGlobal.cpp -o $@ $(CXXFLAGS)

Action/Font/ActionFontEditGlyph.o : Action/Font/ActionFontEditGlyph.cpp
	$(CPP) -c Action/Font/ActionFontEditGlyph.cpp -o $@ $(CXXFLAGS)

Action/Material/ActionMaterialEditAppearance.o : Action/Material/ActionMaterialEditAppearance.cpp
	$(CPP) -c Action/Material/ActionMaterialEditAppearance.cpp -o $@ $(CXXFLAGS)

Action/Material/ActionMaterialEditPhysics.o : Action/Material/ActionMaterialEditPhysics.cpp
	$(CPP) -c Action/Material/ActionMaterialEditPhysics.cpp -o $@ $(CXXFLAGS)

Action/Model/Animation/ActionModelAddAnimation.o : Action/Model/Animation/ActionModelAddAnimation.cpp
	$(CPP) -c Action/Model/Animation/ActionModelAddAnimation.cpp -o $@ $(CXXFLAGS)

Action/Model/Animation/ActionModelAnimationAddFrame.o : Action/Model/Animation/ActionModelAnimationAddFrame.cpp
	$(CPP) -c Action/Model/Animation/ActionModelAnimationAddFrame.cpp -o $@ $(CXXFLAGS)

Action/Model/Animation/ActionModelAnimationDeleteFrame.o : Action/Model/Animation/ActionModelAnimationDeleteFrame.cpp
	$(CPP) -c Action/Model/Animation/ActionModelAnimationDeleteFrame.cpp -o $@ $(CXXFLAGS)

Action/Model/Animation/ActionModelAnimationSetBone.o : Action/Model/Animation/ActionModelAnimationSetBone.cpp
	$(CPP) -c Action/Model/Animation/ActionModelAnimationSetBone.cpp -o $@ $(CXXFLAGS)

Action/Model/Animation/ActionModelAnimationSetData.o : Action/Model/Animation/ActionModelAnimationSetData.cpp
	$(CPP) -c Action/Model/Animation/ActionModelAnimationSetData.cpp -o $@ $(CXXFLAGS)

Action/Model/Animation/ActionModelAnimationSetFrameDuration.o : Action/Model/Animation/ActionModelAnimationSetFrameDuration.cpp
	$(CPP) -c Action/Model/Animation/ActionModelAnimationSetFrameDuration.cpp -o $@ $(CXXFLAGS)

Action/Model/Animation/ActionModelAnimationTransformBones.o : Action/Model/Animation/ActionModelAnimationTransformBones.cpp
	$(CPP) -c Action/Model/Animation/ActionModelAnimationTransformBones.cpp -o $@ $(CXXFLAGS)

Action/Model/Animation/ActionModelAnimationTransformVertices.o : Action/Model/Animation/ActionModelAnimationTransformVertices.cpp
	$(CPP) -c Action/Model/Animation/ActionModelAnimationTransformVertices.cpp -o $@ $(CXXFLAGS)

Action/Model/Animation/ActionModelDeleteAnimation.o : Action/Model/Animation/ActionModelDeleteAnimation.cpp
	$(CPP) -c Action/Model/Animation/ActionModelDeleteAnimation.cpp -o $@ $(CXXFLAGS)

Action/Model/Animation/ActionModelDuplicateAnimation.o : Action/Model/Animation/ActionModelDuplicateAnimation.cpp
	$(CPP) -c Action/Model/Animation/ActionModelDuplicateAnimation.cpp -o $@ $(CXXFLAGS)

Action/Model/Data/ActionModelAddMaterial.o : Action/Model/Data/ActionModelAddMaterial.cpp
	$(CPP) -c Action/Model/Data/ActionModelAddMaterial.cpp -o $@ $(CXXFLAGS)

Action/Model/Data/ActionModelEditData.o : Action/Model/Data/ActionModelEditData.cpp
	$(CPP) -c Action/Model/Data/ActionModelEditData.cpp -o $@ $(CXXFLAGS)

Action/Model/Data/ActionModelEditMaterial.o : Action/Model/Data/ActionModelEditMaterial.cpp
	$(CPP) -c Action/Model/Data/ActionModelEditMaterial.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/ActionModelDeleteSelection.o : Action/Model/Mesh/ActionModelDeleteSelection.cpp
	$(CPP) -c Action/Model/Mesh/ActionModelDeleteSelection.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/ActionModelEasify.o : Action/Model/Mesh/ActionModelEasify.cpp
	$(CPP) -c Action/Model/Mesh/ActionModelEasify.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/ActionModelPasteGeometry.o : Action/Model/Mesh/ActionModelPasteGeometry.cpp
	$(CPP) -c Action/Model/Mesh/ActionModelPasteGeometry.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Brush/ActionModelBrushComplexify.o : Action/Model/Mesh/Brush/ActionModelBrushComplexify.cpp
	$(CPP) -c Action/Model/Mesh/Brush/ActionModelBrushComplexify.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Brush/ActionModelBrushExtrude.o : Action/Model/Mesh/Brush/ActionModelBrushExtrude.cpp
	$(CPP) -c Action/Model/Mesh/Brush/ActionModelBrushExtrude.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Brush/ActionModelBrushSmooth.o : Action/Model/Mesh/Brush/ActionModelBrushSmooth.cpp
	$(CPP) -c Action/Model/Mesh/Brush/ActionModelBrushSmooth.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Edge/ActionModelBevelEdges.o : Action/Model/Mesh/Edge/ActionModelBevelEdges.cpp
	$(CPP) -c Action/Model/Mesh/Edge/ActionModelBevelEdges.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Edge/ActionModelCollapseEdge.o : Action/Model/Mesh/Edge/ActionModelCollapseEdge.cpp
	$(CPP) -c Action/Model/Mesh/Edge/ActionModelCollapseEdge.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Edge/ActionModelSplitEdge.o : Action/Model/Mesh/Edge/ActionModelSplitEdge.cpp
	$(CPP) -c Action/Model/Mesh/Edge/ActionModelSplitEdge.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Effects/ActionModelAddEffects.o : Action/Model/Mesh/Effects/ActionModelAddEffects.cpp
	$(CPP) -c Action/Model/Mesh/Effects/ActionModelAddEffects.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Effects/ActionModelClearEffects.o : Action/Model/Mesh/Effects/ActionModelClearEffects.cpp
	$(CPP) -c Action/Model/Mesh/Effects/ActionModelClearEffects.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Effects/ActionModelEditEffect.o : Action/Model/Mesh/Effects/ActionModelEditEffect.cpp
	$(CPP) -c Action/Model/Mesh/Effects/ActionModelEditEffect.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Look/ActionModelSetMaterial.o : Action/Model/Mesh/Look/ActionModelSetMaterial.cpp
	$(CPP) -c Action/Model/Mesh/Look/ActionModelSetMaterial.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Look/ActionModelSetNormalModeSelection.o : Action/Model/Mesh/Look/ActionModelSetNormalModeSelection.cpp
	$(CPP) -c Action/Model/Mesh/Look/ActionModelSetNormalModeSelection.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Polygon/ActionModelAddPolygonAutoSkin.o : Action/Model/Mesh/Polygon/ActionModelAddPolygonAutoSkin.cpp
	$(CPP) -c Action/Model/Mesh/Polygon/ActionModelAddPolygonAutoSkin.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Polygon/ActionModelAddPolygon.o : Action/Model/Mesh/Polygon/ActionModelAddPolygon.cpp
	$(CPP) -c Action/Model/Mesh/Polygon/ActionModelAddPolygon.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Polygon/ActionModelAddPolygonSingleTexture.o : Action/Model/Mesh/Polygon/ActionModelAddPolygonSingleTexture.cpp
	$(CPP) -c Action/Model/Mesh/Polygon/ActionModelAddPolygonSingleTexture.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Polygon/ActionModelAddPolygonWithSkinGenerator.o : Action/Model/Mesh/Polygon/ActionModelAddPolygonWithSkinGenerator.cpp
	$(CPP) -c Action/Model/Mesh/Polygon/ActionModelAddPolygonWithSkinGenerator.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Polygon/ActionModelCutOutPolygons.o : Action/Model/Mesh/Polygon/ActionModelCutOutPolygons.cpp
	$(CPP) -c Action/Model/Mesh/Polygon/ActionModelCutOutPolygons.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Polygon/ActionModelExtrudePolygons.o : Action/Model/Mesh/Polygon/ActionModelExtrudePolygons.cpp
	$(CPP) -c Action/Model/Mesh/Polygon/ActionModelExtrudePolygons.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Polygon/ActionModelMergePolygonsSelection.o : Action/Model/Mesh/Polygon/ActionModelMergePolygonsSelection.cpp
	$(CPP) -c Action/Model/Mesh/Polygon/ActionModelMergePolygonsSelection.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Polygon/ActionModelSplitPolygon.o : Action/Model/Mesh/Polygon/ActionModelSplitPolygon.cpp
	$(CPP) -c Action/Model/Mesh/Polygon/ActionModelSplitPolygon.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Polygon/ActionModelTriangulateSelection.o : Action/Model/Mesh/Polygon/ActionModelTriangulateSelection.cpp
	$(CPP) -c Action/Model/Mesh/Polygon/ActionModelTriangulateSelection.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Polygon/Helper/ActionModelPolygonAddVertex.o : Action/Model/Mesh/Polygon/Helper/ActionModelPolygonAddVertex.cpp
	$(CPP) -c Action/Model/Mesh/Polygon/Helper/ActionModelPolygonAddVertex.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Polygon/Helper/ActionModelPolygonRemoveVertex.o : Action/Model/Mesh/Polygon/Helper/ActionModelPolygonRemoveVertex.cpp
	$(CPP) -c Action/Model/Mesh/Polygon/Helper/ActionModelPolygonRemoveVertex.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Skin/ActionModelAutomap.o : Action/Model/Mesh/Skin/ActionModelAutomap.cpp
	$(CPP) -c Action/Model/Mesh/Skin/ActionModelAutomap.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Skin/ActionModelSkinVerticesFromProjection.o : Action/Model/Mesh/Skin/ActionModelSkinVerticesFromProjection.cpp
	$(CPP) -c Action/Model/Mesh/Skin/ActionModelSkinVerticesFromProjection.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Skin/ActionModelTransformSkinVertices.o : Action/Model/Mesh/Skin/ActionModelTransformSkinVertices.cpp
	$(CPP) -c Action/Model/Mesh/Skin/ActionModelTransformSkinVertices.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Surface/ActionModelAutoWeldSelection.o : Action/Model/Mesh/Surface/ActionModelAutoWeldSelection.cpp
	$(CPP) -c Action/Model/Mesh/Surface/ActionModelAutoWeldSelection.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Surface/ActionModelDeleteSurface.o : Action/Model/Mesh/Surface/ActionModelDeleteSurface.cpp
	$(CPP) -c Action/Model/Mesh/Surface/ActionModelDeleteSurface.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Surface/ActionModelSurfaceAutoWeld.o : Action/Model/Mesh/Surface/ActionModelSurfaceAutoWeld.cpp
	$(CPP) -c Action/Model/Mesh/Surface/ActionModelSurfaceAutoWeld.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Surface/ActionModelSurfaceCopy.o : Action/Model/Mesh/Surface/ActionModelSurfaceCopy.cpp
	$(CPP) -c Action/Model/Mesh/Surface/ActionModelSurfaceCopy.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Surface/ActionModelSurfaceInvert.o : Action/Model/Mesh/Surface/ActionModelSurfaceInvert.cpp
	$(CPP) -c Action/Model/Mesh/Surface/ActionModelSurfaceInvert.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Surface/ActionModelSurfacesSubdivide.o : Action/Model/Mesh/Surface/ActionModelSurfacesSubdivide.cpp
	$(CPP) -c Action/Model/Mesh/Surface/ActionModelSurfacesSubdivide.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Surface/ActionModelSurfaceVolumeAnd.o : Action/Model/Mesh/Surface/ActionModelSurfaceVolumeAnd.cpp
	$(CPP) -c Action/Model/Mesh/Surface/ActionModelSurfaceVolumeAnd.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Surface/ActionModelSurfaceVolumeSubtract.o : Action/Model/Mesh/Surface/ActionModelSurfaceVolumeSubtract.cpp
	$(CPP) -c Action/Model/Mesh/Surface/ActionModelSurfaceVolumeSubtract.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Surface/Helper/ActionModelAddEmptySurface.o : Action/Model/Mesh/Surface/Helper/ActionModelAddEmptySurface.cpp
	$(CPP) -c Action/Model/Mesh/Surface/Helper/ActionModelAddEmptySurface.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Surface/Helper/ActionModelDeleteEmptySurface.o : Action/Model/Mesh/Surface/Helper/ActionModelDeleteEmptySurface.cpp
	$(CPP) -c Action/Model/Mesh/Surface/Helper/ActionModelDeleteEmptySurface.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Surface/Helper/ActionModelJoinSurfaces.o : Action/Model/Mesh/Surface/Helper/ActionModelJoinSurfaces.cpp
	$(CPP) -c Action/Model/Mesh/Surface/Helper/ActionModelJoinSurfaces.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Surface/Helper/ActionModelSurfaceAddPolygon.o : Action/Model/Mesh/Surface/Helper/ActionModelSurfaceAddPolygon.cpp
	$(CPP) -c Action/Model/Mesh/Surface/Helper/ActionModelSurfaceAddPolygon.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Surface/Helper/ActionModelSurfaceDeletePolygon.o : Action/Model/Mesh/Surface/Helper/ActionModelSurfaceDeletePolygon.cpp
	$(CPP) -c Action/Model/Mesh/Surface/Helper/ActionModelSurfaceDeletePolygon.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Surface/Helper/ActionModelSurfaceRelinkPolygon.o : Action/Model/Mesh/Surface/Helper/ActionModelSurfaceRelinkPolygon.cpp
	$(CPP) -c Action/Model/Mesh/Surface/Helper/ActionModelSurfaceRelinkPolygon.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Vertex/ActionModelAddVertex.o : Action/Model/Mesh/Vertex/ActionModelAddVertex.cpp
	$(CPP) -c Action/Model/Mesh/Vertex/ActionModelAddVertex.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Vertex/ActionModelAlignToGrid.o : Action/Model/Mesh/Vertex/ActionModelAlignToGrid.cpp
	$(CPP) -c Action/Model/Mesh/Vertex/ActionModelAlignToGrid.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Vertex/ActionModelBevelVertices.o : Action/Model/Mesh/Vertex/ActionModelBevelVertices.cpp
	$(CPP) -c Action/Model/Mesh/Vertex/ActionModelBevelVertices.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Vertex/ActionModelCollapseVertices.o : Action/Model/Mesh/Vertex/ActionModelCollapseVertices.cpp
	$(CPP) -c Action/Model/Mesh/Vertex/ActionModelCollapseVertices.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Vertex/ActionModelDeleteVertex.o : Action/Model/Mesh/Vertex/ActionModelDeleteVertex.cpp
	$(CPP) -c Action/Model/Mesh/Vertex/ActionModelDeleteVertex.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Vertex/ActionModelFlattenVertices.o : Action/Model/Mesh/Vertex/ActionModelFlattenVertices.cpp
	$(CPP) -c Action/Model/Mesh/Vertex/ActionModelFlattenVertices.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Vertex/ActionModelNearifyVertices.o : Action/Model/Mesh/Vertex/ActionModelNearifyVertices.cpp
	$(CPP) -c Action/Model/Mesh/Vertex/ActionModelNearifyVertices.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Vertex/ActionModelTransformVertices.o : Action/Model/Mesh/Vertex/ActionModelTransformVertices.cpp
	$(CPP) -c Action/Model/Mesh/Vertex/ActionModelTransformVertices.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Vertex/ActionModelTriangulateVertices.o : Action/Model/Mesh/Vertex/ActionModelTriangulateVertices.cpp
	$(CPP) -c Action/Model/Mesh/Vertex/ActionModelTriangulateVertices.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Vertex/Helper/ActionModelDeleteUnusedVertex.o : Action/Model/Mesh/Vertex/Helper/ActionModelDeleteUnusedVertex.cpp
	$(CPP) -c Action/Model/Mesh/Vertex/Helper/ActionModelDeleteUnusedVertex.cpp -o $@ $(CXXFLAGS)

Action/Model/Mesh/Vertex/Helper/ActionModelMoveVertex.o : Action/Model/Mesh/Vertex/Helper/ActionModelMoveVertex.cpp
	$(CPP) -c Action/Model/Mesh/Vertex/Helper/ActionModelMoveVertex.cpp -o $@ $(CXXFLAGS)

Action/Model/Skeleton/ActionModelAddBone.o : Action/Model/Skeleton/ActionModelAddBone.cpp
	$(CPP) -c Action/Model/Skeleton/ActionModelAddBone.cpp -o $@ $(CXXFLAGS)

Action/Model/Skeleton/ActionModelAttachVerticesToBone.o : Action/Model/Skeleton/ActionModelAttachVerticesToBone.cpp
	$(CPP) -c Action/Model/Skeleton/ActionModelAttachVerticesToBone.cpp -o $@ $(CXXFLAGS)

Action/Model/Skeleton/ActionModelDeleteBone.o : Action/Model/Skeleton/ActionModelDeleteBone.cpp
	$(CPP) -c Action/Model/Skeleton/ActionModelDeleteBone.cpp -o $@ $(CXXFLAGS)

Action/Model/Skeleton/ActionModelDeleteBoneSelection.o : Action/Model/Skeleton/ActionModelDeleteBoneSelection.cpp
	$(CPP) -c Action/Model/Skeleton/ActionModelDeleteBoneSelection.cpp -o $@ $(CXXFLAGS)

Action/Model/Skeleton/ActionModelReconnectBone.o : Action/Model/Skeleton/ActionModelReconnectBone.cpp
	$(CPP) -c Action/Model/Skeleton/ActionModelReconnectBone.cpp -o $@ $(CXXFLAGS)

Action/Model/Skeleton/ActionModelSetSubModel.o : Action/Model/Skeleton/ActionModelSetSubModel.cpp
	$(CPP) -c Action/Model/Skeleton/ActionModelSetSubModel.cpp -o $@ $(CXXFLAGS)

Action/Model/Skeleton/ActionModelTransformBones.o : Action/Model/Skeleton/ActionModelTransformBones.cpp
	$(CPP) -c Action/Model/Skeleton/ActionModelTransformBones.cpp -o $@ $(CXXFLAGS)

Action/World/ActionWorldDeleteSelection.o : Action/World/ActionWorldDeleteSelection.cpp
	$(CPP) -c Action/World/ActionWorldDeleteSelection.cpp -o $@ $(CXXFLAGS)

Action/World/ActionWorldEditData.o : Action/World/ActionWorldEditData.cpp
	$(CPP) -c Action/World/ActionWorldEditData.cpp -o $@ $(CXXFLAGS)

Action/World/ActionWorldMoveSelection.o : Action/World/ActionWorldMoveSelection.cpp
	$(CPP) -c Action/World/ActionWorldMoveSelection.cpp -o $@ $(CXXFLAGS)

Action/World/ActionWorldPaste.o : Action/World/ActionWorldPaste.cpp
	$(CPP) -c Action/World/ActionWorldPaste.cpp -o $@ $(CXXFLAGS)

Action/World/ActionWorldSetEgo.o : Action/World/ActionWorldSetEgo.cpp
	$(CPP) -c Action/World/ActionWorldSetEgo.cpp -o $@ $(CXXFLAGS)

Action/World/Camera/ActionCameraAddPoint.o : Action/World/Camera/ActionCameraAddPoint.cpp
	$(CPP) -c Action/World/Camera/ActionCameraAddPoint.cpp -o $@ $(CXXFLAGS)

Action/World/Camera/ActionCameraMoveSelection.o : Action/World/Camera/ActionCameraMoveSelection.cpp
	$(CPP) -c Action/World/Camera/ActionCameraMoveSelection.cpp -o $@ $(CXXFLAGS)

Action/World/Camera/ActionCameraMoveTimeSelection.o : Action/World/Camera/ActionCameraMoveTimeSelection.cpp
	$(CPP) -c Action/World/Camera/ActionCameraMoveTimeSelection.cpp -o $@ $(CXXFLAGS)

Action/World/Helper/ActionWorldTerrainEditHeight.o : Action/World/Helper/ActionWorldTerrainEditHeight.cpp
	$(CPP) -c Action/World/Helper/ActionWorldTerrainEditHeight.cpp -o $@ $(CXXFLAGS)

Action/World/Object/ActionWorldAddObject.o : Action/World/Object/ActionWorldAddObject.cpp
	$(CPP) -c Action/World/Object/ActionWorldAddObject.cpp -o $@ $(CXXFLAGS)

Action/World/Object/ActionWorldDeleteObject.o : Action/World/Object/ActionWorldDeleteObject.cpp
	$(CPP) -c Action/World/Object/ActionWorldDeleteObject.cpp -o $@ $(CXXFLAGS)

Action/World/Object/ActionWorldEditObject.o : Action/World/Object/ActionWorldEditObject.cpp
	$(CPP) -c Action/World/Object/ActionWorldEditObject.cpp -o $@ $(CXXFLAGS)

Action/World/Object/ActionWorldRotateObjects.o : Action/World/Object/ActionWorldRotateObjects.cpp
	$(CPP) -c Action/World/Object/ActionWorldRotateObjects.cpp -o $@ $(CXXFLAGS)

Action/World/Terrain/ActionWorldAddTerrain.o : Action/World/Terrain/ActionWorldAddTerrain.cpp
	$(CPP) -c Action/World/Terrain/ActionWorldAddTerrain.cpp -o $@ $(CXXFLAGS)

Action/World/Terrain/ActionWorldDeleteTerrain.o : Action/World/Terrain/ActionWorldDeleteTerrain.cpp
	$(CPP) -c Action/World/Terrain/ActionWorldDeleteTerrain.cpp -o $@ $(CXXFLAGS)

Action/World/Terrain/ActionWorldEditTerrain.o : Action/World/Terrain/ActionWorldEditTerrain.cpp
	$(CPP) -c Action/World/Terrain/ActionWorldEditTerrain.cpp -o $@ $(CXXFLAGS)

Action/World/Terrain/ActionWorldTerrainApplyHeightmap.o : Action/World/Terrain/ActionWorldTerrainApplyHeightmap.cpp
	$(CPP) -c Action/World/Terrain/ActionWorldTerrainApplyHeightmap.cpp -o $@ $(CXXFLAGS)

Action/World/Terrain/ActionWorldTerrainBrushExtrude.o : Action/World/Terrain/ActionWorldTerrainBrushExtrude.cpp
	$(CPP) -c Action/World/Terrain/ActionWorldTerrainBrushExtrude.cpp -o $@ $(CXXFLAGS)

Data/Administration/AdminFile.o : Data/Administration/AdminFile.cpp
	$(CPP) -c Data/Administration/AdminFile.cpp -o $@ $(CXXFLAGS)

Data/Administration/AdminFileList.o : Data/Administration/AdminFileList.cpp
	$(CPP) -c Data/Administration/AdminFileList.cpp -o $@ $(CXXFLAGS)

Data/Administration/DataAdministration.o : Data/Administration/DataAdministration.cpp
	$(CPP) -c Data/Administration/DataAdministration.cpp -o $@ $(CXXFLAGS)

Data/Administration/GameIniData.o : Data/Administration/GameIniData.cpp
	$(CPP) -c Data/Administration/GameIniData.cpp -o $@ $(CXXFLAGS)

Data/Data.o : Data/Data.cpp
	$(CPP) -c Data/Data.cpp -o $@ $(CXXFLAGS)

Data/Font/DataFont.o : Data/Font/DataFont.cpp
	$(CPP) -c Data/Font/DataFont.cpp -o $@ $(CXXFLAGS)

Data/Font/Import/ImporterCairo.o : Data/Font/Import/ImporterCairo.cpp
	$(CPP) -c Data/Font/Import/ImporterCairo.cpp -o $@ $(CXXFLAGS)

Data/Material/DataMaterial.o : Data/Material/DataMaterial.cpp
	$(CPP) -c Data/Material/DataMaterial.cpp -o $@ $(CXXFLAGS)

Data/Model/BspTree.o : Data/Model/BspTree.cpp
	$(CPP) -c Data/Model/BspTree.cpp -o $@ $(CXXFLAGS)

Data/Model/DataModel.o : Data/Model/DataModel.cpp
	$(CPP) -c Data/Model/DataModel.cpp -o $@ $(CXXFLAGS)

Data/Model/Export/ExporterJson.o : Data/Model/Export/ExporterJson.cpp
	$(CPP) -c Data/Model/Export/ExporterJson.cpp -o $@ $(CXXFLAGS)

Data/Model/Geometry/GeometryBall.o : Data/Model/Geometry/GeometryBall.cpp
	$(CPP) -c Data/Model/Geometry/GeometryBall.cpp -o $@ $(CXXFLAGS)

Data/Model/Geometry/Geometry.o : Data/Model/Geometry/Geometry.cpp
	$(CPP) -c Data/Model/Geometry/Geometry.cpp -o $@ $(CXXFLAGS)

Data/Model/Geometry/GeometryCube.o : Data/Model/Geometry/GeometryCube.cpp
	$(CPP) -c Data/Model/Geometry/GeometryCube.cpp -o $@ $(CXXFLAGS)

Data/Model/Geometry/GeometryCylinder.o : Data/Model/Geometry/GeometryCylinder.cpp
	$(CPP) -c Data/Model/Geometry/GeometryCylinder.cpp -o $@ $(CXXFLAGS)

Data/Model/Geometry/GeometryPlane.o : Data/Model/Geometry/GeometryPlane.cpp
	$(CPP) -c Data/Model/Geometry/GeometryPlane.cpp -o $@ $(CXXFLAGS)

Data/Model/Geometry/GeometryPlatonic.o : Data/Model/Geometry/GeometryPlatonic.cpp
	$(CPP) -c Data/Model/Geometry/GeometryPlatonic.cpp -o $@ $(CXXFLAGS)

Data/Model/Geometry/GeometrySphere.o : Data/Model/Geometry/GeometrySphere.cpp
	$(CPP) -c Data/Model/Geometry/GeometrySphere.cpp -o $@ $(CXXFLAGS)

Data/Model/Geometry/GeometrySubtract.o : Data/Model/Geometry/GeometrySubtract.cpp
	$(CPP) -c Data/Model/Geometry/GeometrySubtract.cpp -o $@ $(CXXFLAGS)

Data/Model/Geometry/GeometryTeapot.o : Data/Model/Geometry/GeometryTeapot.cpp
	$(CPP) -c Data/Model/Geometry/GeometryTeapot.cpp -o $@ $(CXXFLAGS)

Data/Model/Geometry/GeometryTorus.o : Data/Model/Geometry/GeometryTorus.cpp
	$(CPP) -c Data/Model/Geometry/GeometryTorus.cpp -o $@ $(CXXFLAGS)

Data/Model/Geometry/GeometryTorusKnot.o : Data/Model/Geometry/GeometryTorusKnot.cpp
	$(CPP) -c Data/Model/Geometry/GeometryTorusKnot.cpp -o $@ $(CXXFLAGS)

Data/Model/Import/Importer3ds.o : Data/Model/Import/Importer3ds.cpp
	$(CPP) -c Data/Model/Import/Importer3ds.cpp -o $@ $(CXXFLAGS)

Data/Model/Import/ImporterJson.o : Data/Model/Import/ImporterJson.cpp
	$(CPP) -c Data/Model/Import/ImporterJson.cpp -o $@ $(CXXFLAGS)

Data/Model/Import/ImporterPly.o : Data/Model/Import/ImporterPly.cpp
	$(CPP) -c Data/Model/Import/ImporterPly.cpp -o $@ $(CXXFLAGS)

Data/Model/ModelMaterial.o : Data/Model/ModelMaterial.cpp
	$(CPP) -c Data/Model/ModelMaterial.cpp -o $@ $(CXXFLAGS)

Data/Model/ModelPolygon.o : Data/Model/ModelPolygon.cpp
	$(CPP) -c Data/Model/ModelPolygon.cpp -o $@ $(CXXFLAGS)

Data/Model/ModelSurface.o : Data/Model/ModelSurface.cpp
	$(CPP) -c Data/Model/ModelSurface.cpp -o $@ $(CXXFLAGS)

Data/Model/SkinGenerator.o : Data/Model/SkinGenerator.cpp
	$(CPP) -c Data/Model/SkinGenerator.cpp -o $@ $(CXXFLAGS)

Data/World/DataCamera.o : Data/World/DataCamera.cpp
	$(CPP) -c Data/World/DataCamera.cpp -o $@ $(CXXFLAGS)

Data/World/DataWorld.o : Data/World/DataWorld.cpp
	$(CPP) -c Data/World/DataWorld.cpp -o $@ $(CXXFLAGS)

Data/World/Lightmap/Lightmap.o : Data/World/Lightmap/Lightmap.cpp
	$(CPP) -c Data/World/Lightmap/Lightmap.cpp -o $@ $(CXXFLAGS)

Data/World/Lightmap/LightmapData.o : Data/World/Lightmap/LightmapData.cpp
	$(CPP) -c Data/World/Lightmap/LightmapData.cpp -o $@ $(CXXFLAGS)

Data/World/Lightmap/LightmapPhotonMap.o : Data/World/Lightmap/LightmapPhotonMap.cpp
	$(CPP) -c Data/World/Lightmap/LightmapPhotonMap.cpp -o $@ $(CXXFLAGS)

Data/World/Lightmap/LightmapPhotonMapImageSpace.o : Data/World/Lightmap/LightmapPhotonMapImageSpace.cpp
	$(CPP) -c Data/World/Lightmap/LightmapPhotonMapImageSpace.cpp -o $@ $(CXXFLAGS)

Data/World/Lightmap/LightmapRadiosity.o : Data/World/Lightmap/LightmapRadiosity.cpp
	$(CPP) -c Data/World/Lightmap/LightmapRadiosity.cpp -o $@ $(CXXFLAGS)

Data/World/Lightmap/LightmapRayTracing.o : Data/World/Lightmap/LightmapRayTracing.cpp
	$(CPP) -c Data/World/Lightmap/LightmapRayTracing.cpp -o $@ $(CXXFLAGS)

Edward.o : Edward.cpp
	$(CPP) -c Edward.cpp -o $@ $(CXXFLAGS)

lib/base/array.o : lib/base/array.cpp
	$(CPP) -c lib/base/array.cpp -o $@ $(CXXFLAGS)

lib/base/strings.o : lib/base/strings.cpp
	$(CPP) -c lib/base/strings.cpp -o $@ $(CXXFLAGS)

lib/file/file.o : lib/file/file.cpp
	$(CPP) -c lib/file/file.cpp -o $@ $(CXXFLAGS)

lib/file/file_op.o : lib/file/file_op.cpp
	$(CPP) -c lib/file/file_op.cpp -o $@ $(CXXFLAGS)

lib/file/msg.o : lib/file/msg.cpp
	$(CPP) -c lib/file/msg.cpp -o $@ $(CXXFLAGS)

lib/hui/Application.o : lib/hui/Application.cpp
	$(CPP) -c lib/hui/Application.cpp -o $@ $(CXXFLAGS)

lib/hui/Callback.o : lib/hui/Callback.cpp
	$(CPP) -c lib/hui/Callback.cpp -o $@ $(CXXFLAGS)

lib/hui/Clipboard.o : lib/hui/Clipboard.cpp
	$(CPP) -c lib/hui/Clipboard.cpp -o $@ $(CXXFLAGS)

lib/hui/common_dlg.o : lib/hui/common_dlg.cpp
	$(CPP) -c lib/hui/common_dlg.cpp -o $@ $(CXXFLAGS)

lib/hui/common_dlg_gtk.o : lib/hui/common_dlg_gtk.cpp
	$(CPP) -c lib/hui/common_dlg_gtk.cpp -o $@ $(CXXFLAGS)

lib/hui/common_dlg_win.o : lib/hui/common_dlg_win.cpp
	$(CPP) -c lib/hui/common_dlg_win.cpp -o $@ $(CXXFLAGS)

lib/hui/Config.o : lib/hui/Config.cpp
	$(CPP) -c lib/hui/Config.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlButtonGtk.o : lib/hui/Controls/ControlButtonGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlButtonGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlCheckBoxGtk.o : lib/hui/Controls/ControlCheckBoxGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlCheckBoxGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlColorButtonGtk.o : lib/hui/Controls/ControlColorButtonGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlColorButtonGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlComboBoxGtk.o : lib/hui/Controls/ControlComboBoxGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlComboBoxGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/Control.o : lib/hui/Controls/Control.cpp
	$(CPP) -c lib/hui/Controls/Control.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlDrawingAreaGtk.o : lib/hui/Controls/ControlDrawingAreaGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlDrawingAreaGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlEditGtk.o : lib/hui/Controls/ControlEditGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlEditGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlExpanderGtk.o : lib/hui/Controls/ControlExpanderGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlExpanderGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlGridGtk.o : lib/hui/Controls/ControlGridGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlGridGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlGroupGtk.o : lib/hui/Controls/ControlGroupGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlGroupGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlLabelGtk.o : lib/hui/Controls/ControlLabelGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlLabelGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlListViewGtk.o : lib/hui/Controls/ControlListViewGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlListViewGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlMultilineEditGtk.o : lib/hui/Controls/ControlMultilineEditGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlMultilineEditGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlPanedGtk.o : lib/hui/Controls/ControlPanedGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlPanedGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlProgressBarGtk.o : lib/hui/Controls/ControlProgressBarGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlProgressBarGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlRadioButtonGtk.o : lib/hui/Controls/ControlRadioButtonGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlRadioButtonGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlRevealerGtk.o : lib/hui/Controls/ControlRevealerGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlRevealerGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlScrollerGtk.o : lib/hui/Controls/ControlScrollerGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlScrollerGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlSeparatorGtk.o : lib/hui/Controls/ControlSeparatorGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlSeparatorGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlSliderGtk.o : lib/hui/Controls/ControlSliderGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlSliderGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlSpinButtonGtk.o : lib/hui/Controls/ControlSpinButtonGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlSpinButtonGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlTabControlGtk.o : lib/hui/Controls/ControlTabControlGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlTabControlGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlToggleButtonGtk.o : lib/hui/Controls/ControlToggleButtonGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlToggleButtonGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlTreeViewGtk.o : lib/hui/Controls/ControlTreeViewGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlTreeViewGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/MenuItemGtk.o : lib/hui/Controls/MenuItemGtk.cpp
	$(CPP) -c lib/hui/Controls/MenuItemGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/MenuItemSeparatorGtk.o : lib/hui/Controls/MenuItemSeparatorGtk.cpp
	$(CPP) -c lib/hui/Controls/MenuItemSeparatorGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/MenuItemSeparatorWin.o : lib/hui/Controls/MenuItemSeparatorWin.cpp
	$(CPP) -c lib/hui/Controls/MenuItemSeparatorWin.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/MenuItemSubmenuGtk.o : lib/hui/Controls/MenuItemSubmenuGtk.cpp
	$(CPP) -c lib/hui/Controls/MenuItemSubmenuGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/MenuItemSubmenuWin.o : lib/hui/Controls/MenuItemSubmenuWin.cpp
	$(CPP) -c lib/hui/Controls/MenuItemSubmenuWin.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/MenuItemToggleGtk.o : lib/hui/Controls/MenuItemToggleGtk.cpp
	$(CPP) -c lib/hui/Controls/MenuItemToggleGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/MenuItemToggleWin.o : lib/hui/Controls/MenuItemToggleWin.cpp
	$(CPP) -c lib/hui/Controls/MenuItemToggleWin.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/MenuItemWin.o : lib/hui/Controls/MenuItemWin.cpp
	$(CPP) -c lib/hui/Controls/MenuItemWin.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ToolItemButtonGtk.o : lib/hui/Controls/ToolItemButtonGtk.cpp
	$(CPP) -c lib/hui/Controls/ToolItemButtonGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ToolItemButtonWin.o : lib/hui/Controls/ToolItemButtonWin.cpp
	$(CPP) -c lib/hui/Controls/ToolItemButtonWin.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ToolItemMenuButtonGtk.o : lib/hui/Controls/ToolItemMenuButtonGtk.cpp
	$(CPP) -c lib/hui/Controls/ToolItemMenuButtonGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ToolItemMenuButtonWin.o : lib/hui/Controls/ToolItemMenuButtonWin.cpp
	$(CPP) -c lib/hui/Controls/ToolItemMenuButtonWin.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ToolItemSeparatorGtk.o : lib/hui/Controls/ToolItemSeparatorGtk.cpp
	$(CPP) -c lib/hui/Controls/ToolItemSeparatorGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ToolItemSeparatorWin.o : lib/hui/Controls/ToolItemSeparatorWin.cpp
	$(CPP) -c lib/hui/Controls/ToolItemSeparatorWin.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ToolItemToggleButtonGtk.o : lib/hui/Controls/ToolItemToggleButtonGtk.cpp
	$(CPP) -c lib/hui/Controls/ToolItemToggleButtonGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ToolItemToggleButtonWin.o : lib/hui/Controls/ToolItemToggleButtonWin.cpp
	$(CPP) -c lib/hui/Controls/ToolItemToggleButtonWin.cpp -o $@ $(CXXFLAGS)

lib/hui/error.o : lib/hui/error.cpp
	$(CPP) -c lib/hui/error.cpp -o $@ $(CXXFLAGS)

lib/hui/Event.o : lib/hui/Event.cpp
	$(CPP) -c lib/hui/Event.cpp -o $@ $(CXXFLAGS)

lib/hui/hui.o : lib/hui/hui.cpp
	$(CPP) -c lib/hui/hui.cpp -o $@ $(CXXFLAGS)

lib/hui/language.o : lib/hui/language.cpp
	$(CPP) -c lib/hui/language.cpp -o $@ $(CXXFLAGS)

lib/hui/main_gtk.o : lib/hui/main_gtk.cpp
	$(CPP) -c lib/hui/main_gtk.cpp -o $@ $(CXXFLAGS)

lib/hui/main_win.o : lib/hui/main_win.cpp
	$(CPP) -c lib/hui/main_win.cpp -o $@ $(CXXFLAGS)

lib/hui/Menu.o : lib/hui/Menu.cpp
	$(CPP) -c lib/hui/Menu.cpp -o $@ $(CXXFLAGS)

lib/hui/MenuGtk.o : lib/hui/MenuGtk.cpp
	$(CPP) -c lib/hui/MenuGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/MenuWin.o : lib/hui/MenuWin.cpp
	$(CPP) -c lib/hui/MenuWin.cpp -o $@ $(CXXFLAGS)

lib/hui/PainterGtk.o : lib/hui/PainterGtk.cpp
	$(CPP) -c lib/hui/PainterGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Panel.o : lib/hui/Panel.cpp
	$(CPP) -c lib/hui/Panel.cpp -o $@ $(CXXFLAGS)

lib/hui/Resource.o : lib/hui/Resource.cpp
	$(CPP) -c lib/hui/Resource.cpp -o $@ $(CXXFLAGS)

lib/hui/Timer.o : lib/hui/Timer.cpp
	$(CPP) -c lib/hui/Timer.cpp -o $@ $(CXXFLAGS)

lib/hui/Toolbar.o : lib/hui/Toolbar.cpp
	$(CPP) -c lib/hui/Toolbar.cpp -o $@ $(CXXFLAGS)

lib/hui/ToolbarGtk.o : lib/hui/ToolbarGtk.cpp
	$(CPP) -c lib/hui/ToolbarGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/ToolbarWin.o : lib/hui/ToolbarWin.cpp
	$(CPP) -c lib/hui/ToolbarWin.cpp -o $@ $(CXXFLAGS)

lib/hui/utility.o : lib/hui/utility.cpp
	$(CPP) -c lib/hui/utility.cpp -o $@ $(CXXFLAGS)

lib/hui/window_control.o : lib/hui/window_control.cpp
	$(CPP) -c lib/hui/window_control.cpp -o $@ $(CXXFLAGS)

lib/hui/window_control_gtk.o : lib/hui/window_control_gtk.cpp
	$(CPP) -c lib/hui/window_control_gtk.cpp -o $@ $(CXXFLAGS)

lib/hui/window_control_win.o : lib/hui/window_control_win.cpp
	$(CPP) -c lib/hui/window_control_win.cpp -o $@ $(CXXFLAGS)

lib/hui/Window.o : lib/hui/Window.cpp
	$(CPP) -c lib/hui/Window.cpp -o $@ $(CXXFLAGS)

lib/hui/WindowGtk.o : lib/hui/WindowGtk.cpp
	$(CPP) -c lib/hui/WindowGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/WindowWin.o : lib/hui/WindowWin.cpp
	$(CPP) -c lib/hui/WindowWin.cpp -o $@ $(CXXFLAGS)

lib/image/color.o : lib/image/color.cpp
	$(CPP) -c lib/image/color.cpp -o $@ $(CXXFLAGS)

lib/image/image_bmp.o : lib/image/image_bmp.cpp
	$(CPP) -c lib/image/image_bmp.cpp -o $@ $(CXXFLAGS)

lib/image/image.o : lib/image/image.cpp
	$(CPP) -c lib/image/image.cpp -o $@ $(CXXFLAGS)

lib/image/image_jpg.o : lib/image/image_jpg.cpp
	$(CPP) -c lib/image/image_jpg.cpp -o $@ $(CXXFLAGS)

lib/image/ImagePainter.o : lib/image/ImagePainter.cpp
	$(CPP) -c lib/image/ImagePainter.cpp -o $@ $(CXXFLAGS)

lib/image/image_png.o : lib/image/image_png.cpp
	$(CPP) -c lib/image/image_png.cpp -o $@ $(CXXFLAGS)

lib/image/image_tga.o : lib/image/image_tga.cpp
	$(CPP) -c lib/image/image_tga.cpp -o $@ $(CXXFLAGS)

lib/kaba/asm/asm.o : lib/kaba/asm/asm.cpp
	$(CPP) -c lib/kaba/asm/asm.cpp -o $@ $(CXXFLAGS)

lib/kaba/compiler/compiler.o : lib/kaba/compiler/compiler.cpp
	$(CPP) -c lib/kaba/compiler/compiler.cpp -o $@ $(CXXFLAGS)

lib/kaba/compiler/mapper.o : lib/kaba/compiler/mapper.cpp
	$(CPP) -c lib/kaba/compiler/mapper.cpp -o $@ $(CXXFLAGS)

lib/kaba/compiler/serializer_amd64.o : lib/kaba/compiler/serializer_amd64.cpp
	$(CPP) -c lib/kaba/compiler/serializer_amd64.cpp -o $@ $(CXXFLAGS)

lib/kaba/compiler/serializer_arm.o : lib/kaba/compiler/serializer_arm.cpp
	$(CPP) -c lib/kaba/compiler/serializer_arm.cpp -o $@ $(CXXFLAGS)

lib/kaba/compiler/serializer.o : lib/kaba/compiler/serializer.cpp
	$(CPP) -c lib/kaba/compiler/serializer.cpp -o $@ $(CXXFLAGS)

lib/kaba/compiler/serializer_x86.o : lib/kaba/compiler/serializer_x86.cpp
	$(CPP) -c lib/kaba/compiler/serializer_x86.cpp -o $@ $(CXXFLAGS)

lib/kaba/kaba.o : lib/kaba/kaba.cpp
	$(CPP) -c lib/kaba/kaba.cpp -o $@ $(CXXFLAGS)

lib/kaba/lib/exception.o : lib/kaba/lib/exception.cpp
	$(CPP) -c lib/kaba/lib/exception.cpp -o $@ $(CXXFLAGS)

lib/kaba/lib/lib.o : lib/kaba/lib/lib.cpp
	$(CPP) -c lib/kaba/lib/lib.cpp -o $@ $(CXXFLAGS)

lib/kaba/lib/lib_file.o : lib/kaba/lib/lib_file.cpp
	$(CPP) -c lib/kaba/lib/lib_file.cpp -o $@ $(CXXFLAGS)

lib/kaba/lib/lib_hui.o : lib/kaba/lib/lib_hui.cpp
	$(CPP) -c lib/kaba/lib/lib_hui.cpp -o $@ $(CXXFLAGS)

lib/kaba/lib/lib_image.o : lib/kaba/lib/lib_image.cpp
	$(CPP) -c lib/kaba/lib/lib_image.cpp -o $@ $(CXXFLAGS)

lib/kaba/lib/lib_math.o : lib/kaba/lib/lib_math.cpp
	$(CPP) -c lib/kaba/lib/lib_math.cpp -o $@ $(CXXFLAGS)

lib/kaba/lib/lib_net.o : lib/kaba/lib/lib_net.cpp
	$(CPP) -c lib/kaba/lib/lib_net.cpp -o $@ $(CXXFLAGS)

lib/kaba/lib/lib_nix.o : lib/kaba/lib/lib_nix.cpp
	$(CPP) -c lib/kaba/lib/lib_nix.cpp -o $@ $(CXXFLAGS)

lib/kaba/lib/lib_sound.o : lib/kaba/lib/lib_sound.cpp
	$(CPP) -c lib/kaba/lib/lib_sound.cpp -o $@ $(CXXFLAGS)

lib/kaba/lib/lib_threads.o : lib/kaba/lib/lib_threads.cpp
	$(CPP) -c lib/kaba/lib/lib_threads.cpp -o $@ $(CXXFLAGS)

lib/kaba/lib/lib_x.o : lib/kaba/lib/lib_x.cpp
	$(CPP) -c lib/kaba/lib/lib_x.cpp -o $@ $(CXXFLAGS)

lib/kaba/syntax/class.o : lib/kaba/syntax/class.cpp
	$(CPP) -c lib/kaba/syntax/class.cpp -o $@ $(CXXFLAGS)

lib/kaba/syntax/implicit.o : lib/kaba/syntax/implicit.cpp
	$(CPP) -c lib/kaba/syntax/implicit.cpp -o $@ $(CXXFLAGS)

lib/kaba/syntax/lexical.o : lib/kaba/syntax/lexical.cpp
	$(CPP) -c lib/kaba/syntax/lexical.cpp -o $@ $(CXXFLAGS)

lib/kaba/syntax/parser.o : lib/kaba/syntax/parser.cpp
	$(CPP) -c lib/kaba/syntax/parser.cpp -o $@ $(CXXFLAGS)

lib/kaba/syntax/precompiler.o : lib/kaba/syntax/precompiler.cpp
	$(CPP) -c lib/kaba/syntax/precompiler.cpp -o $@ $(CXXFLAGS)

lib/kaba/syntax/preprocessor.o : lib/kaba/syntax/preprocessor.cpp
	$(CPP) -c lib/kaba/syntax/preprocessor.cpp -o $@ $(CXXFLAGS)

lib/kaba/syntax/syntax_tree.o : lib/kaba/syntax/syntax_tree.cpp
	$(CPP) -c lib/kaba/syntax/syntax_tree.cpp -o $@ $(CXXFLAGS)

lib/math/complex.o : lib/math/complex.cpp
	$(CPP) -c lib/math/complex.cpp -o $@ $(CXXFLAGS)

lib/math/interpolation.o : lib/math/interpolation.cpp
	$(CPP) -c lib/math/interpolation.cpp -o $@ $(CXXFLAGS)

lib/math/math.o : lib/math/math.cpp
	$(CPP) -c lib/math/math.cpp -o $@ $(CXXFLAGS)

lib/math/matrix3.o : lib/math/matrix3.cpp
	$(CPP) -c lib/math/matrix3.cpp -o $@ $(CXXFLAGS)

lib/math/matrix.o : lib/math/matrix.cpp
	$(CPP) -c lib/math/matrix.cpp -o $@ $(CXXFLAGS)

lib/math/plane.o : lib/math/plane.cpp
	$(CPP) -c lib/math/plane.cpp -o $@ $(CXXFLAGS)

lib/math/quaternion.o : lib/math/quaternion.cpp
	$(CPP) -c lib/math/quaternion.cpp -o $@ $(CXXFLAGS)

lib/math/random.o : lib/math/random.cpp
	$(CPP) -c lib/math/random.cpp -o $@ $(CXXFLAGS)

lib/math/ray.o : lib/math/ray.cpp
	$(CPP) -c lib/math/ray.cpp -o $@ $(CXXFLAGS)

lib/math/rect.o : lib/math/rect.cpp
	$(CPP) -c lib/math/rect.cpp -o $@ $(CXXFLAGS)

lib/math/vector.o : lib/math/vector.cpp
	$(CPP) -c lib/math/vector.cpp -o $@ $(CXXFLAGS)

lib/net/net.o : lib/net/net.cpp
	$(CPP) -c lib/net/net.cpp -o $@ $(CXXFLAGS)

lib/nix/nix.o : lib/nix/nix.cpp
	$(CPP) -c lib/nix/nix.cpp -o $@ $(CXXFLAGS)

lib/nix/nix_draw.o : lib/nix/nix_draw.cpp
	$(CPP) -c lib/nix/nix_draw.cpp -o $@ $(CXXFLAGS)

lib/nix/nix_light.o : lib/nix/nix_light.cpp
	$(CPP) -c lib/nix/nix_light.cpp -o $@ $(CXXFLAGS)

lib/nix/nix_shader.o : lib/nix/nix_shader.cpp
	$(CPP) -c lib/nix/nix_shader.cpp -o $@ $(CXXFLAGS)

lib/nix/nix_textures.o : lib/nix/nix_textures.cpp
	$(CPP) -c lib/nix/nix_textures.cpp -o $@ $(CXXFLAGS)

lib/nix/nix_vertexbuffer.o : lib/nix/nix_vertexbuffer.cpp
	$(CPP) -c lib/nix/nix_vertexbuffer.cpp -o $@ $(CXXFLAGS)

lib/nix/nix_view.o : lib/nix/nix_view.cpp
	$(CPP) -c lib/nix/nix_view.cpp -o $@ $(CXXFLAGS)

lib/threads/Mutex.o : lib/threads/Mutex.cpp
	$(CPP) -c lib/threads/Mutex.cpp -o $@ $(CXXFLAGS)

lib/threads/Thread.o : lib/threads/Thread.cpp
	$(CPP) -c lib/threads/Thread.cpp -o $@ $(CXXFLAGS)

lib/threads/ThreadedWork.o : lib/threads/ThreadedWork.cpp
	$(CPP) -c lib/threads/ThreadedWork.cpp -o $@ $(CXXFLAGS)

main.o : main.cpp
	$(CPP) -c main.cpp -o $@ $(CXXFLAGS)

meta.o : meta.cpp
	$(CPP) -c meta.cpp -o $@ $(CXXFLAGS)

Mode/Administration/Dialog/AdministrationDialog.o : Mode/Administration/Dialog/AdministrationDialog.cpp
	$(CPP) -c Mode/Administration/Dialog/AdministrationDialog.cpp -o $@ $(CXXFLAGS)

Mode/Administration/Dialog/ConfigurationDialog.o : Mode/Administration/Dialog/ConfigurationDialog.cpp
	$(CPP) -c Mode/Administration/Dialog/ConfigurationDialog.cpp -o $@ $(CXXFLAGS)

Mode/Administration/ModeAdministration.o : Mode/Administration/ModeAdministration.cpp
	$(CPP) -c Mode/Administration/ModeAdministration.cpp -o $@ $(CXXFLAGS)

Mode/Font/Dialog/FontDialog.o : Mode/Font/Dialog/FontDialog.cpp
	$(CPP) -c Mode/Font/Dialog/FontDialog.cpp -o $@ $(CXXFLAGS)

Mode/Font/ModeFont.o : Mode/Font/ModeFont.cpp
	$(CPP) -c Mode/Font/ModeFont.cpp -o $@ $(CXXFLAGS)

Mode/Material/Dialog/MaterialPropertiesDialog.o : Mode/Material/Dialog/MaterialPropertiesDialog.cpp
	$(CPP) -c Mode/Material/Dialog/MaterialPropertiesDialog.cpp -o $@ $(CXXFLAGS)

Mode/Material/ModeMaterial.o : Mode/Material/ModeMaterial.cpp
	$(CPP) -c Mode/Material/ModeMaterial.cpp -o $@ $(CXXFLAGS)

Mode/Mode.o : Mode/Mode.cpp
	$(CPP) -c Mode/Mode.cpp -o $@ $(CXXFLAGS)

Mode/ModeCreation.o : Mode/ModeCreation.cpp
	$(CPP) -c Mode/ModeCreation.cpp -o $@ $(CXXFLAGS)

Mode/Model/Animation/Creation/ModeModelAnimationInterpolateFrames.o : Mode/Model/Animation/Creation/ModeModelAnimationInterpolateFrames.cpp
	$(CPP) -c Mode/Model/Animation/Creation/ModeModelAnimationInterpolateFrames.cpp -o $@ $(CXXFLAGS)

Mode/Model/Animation/ModeModelAnimation.o : Mode/Model/Animation/ModeModelAnimation.cpp
	$(CPP) -c Mode/Model/Animation/ModeModelAnimation.cpp -o $@ $(CXXFLAGS)

Mode/Model/Animation/ModeModelAnimationNone.o : Mode/Model/Animation/ModeModelAnimationNone.cpp
	$(CPP) -c Mode/Model/Animation/ModeModelAnimationNone.cpp -o $@ $(CXXFLAGS)

Mode/Model/Animation/ModeModelAnimationSkeleton.o : Mode/Model/Animation/ModeModelAnimationSkeleton.cpp
	$(CPP) -c Mode/Model/Animation/ModeModelAnimationSkeleton.cpp -o $@ $(CXXFLAGS)

Mode/Model/Animation/ModeModelAnimationVertex.o : Mode/Model/Animation/ModeModelAnimationVertex.cpp
	$(CPP) -c Mode/Model/Animation/ModeModelAnimationVertex.cpp -o $@ $(CXXFLAGS)

Mode/Model/Dialog/ModelAnimationDialog.o : Mode/Model/Dialog/ModelAnimationDialog.cpp
	$(CPP) -c Mode/Model/Dialog/ModelAnimationDialog.cpp -o $@ $(CXXFLAGS)

Mode/Model/Dialog/ModelAnimationTimelinePanel.o : Mode/Model/Dialog/ModelAnimationTimelinePanel.cpp
	$(CPP) -c Mode/Model/Dialog/ModelAnimationTimelinePanel.cpp -o $@ $(CXXFLAGS)

Mode/Model/Dialog/ModelDuplicateAnimationDialog.o : Mode/Model/Dialog/ModelDuplicateAnimationDialog.cpp
	$(CPP) -c Mode/Model/Dialog/ModelDuplicateAnimationDialog.cpp -o $@ $(CXXFLAGS)

Mode/Model/Dialog/ModelEasifyDialog.o : Mode/Model/Dialog/ModelEasifyDialog.cpp
	$(CPP) -c Mode/Model/Dialog/ModelEasifyDialog.cpp -o $@ $(CXXFLAGS)

Mode/Model/Dialog/ModelFXDialog.o : Mode/Model/Dialog/ModelFXDialog.cpp
	$(CPP) -c Mode/Model/Dialog/ModelFXDialog.cpp -o $@ $(CXXFLAGS)

Mode/Model/Dialog/ModelMaterialDialog.o : Mode/Model/Dialog/ModelMaterialDialog.cpp
	$(CPP) -c Mode/Model/Dialog/ModelMaterialDialog.cpp -o $@ $(CXXFLAGS)

Mode/Model/Dialog/ModelMaterialSelectionDialog.o : Mode/Model/Dialog/ModelMaterialSelectionDialog.cpp
	$(CPP) -c Mode/Model/Dialog/ModelMaterialSelectionDialog.cpp -o $@ $(CXXFLAGS)

Mode/Model/Dialog/ModelNewAnimationDialog.o : Mode/Model/Dialog/ModelNewAnimationDialog.cpp
	$(CPP) -c Mode/Model/Dialog/ModelNewAnimationDialog.cpp -o $@ $(CXXFLAGS)

Mode/Model/Dialog/ModelPropertiesDialog.o : Mode/Model/Dialog/ModelPropertiesDialog.cpp
	$(CPP) -c Mode/Model/Dialog/ModelPropertiesDialog.cpp -o $@ $(CXXFLAGS)

Mode/Model/Mesh/Creation/ModeModelMeshAutoweld.o : Mode/Model/Mesh/Creation/ModeModelMeshAutoweld.cpp
	$(CPP) -c Mode/Model/Mesh/Creation/ModeModelMeshAutoweld.cpp -o $@ $(CXXFLAGS)

Mode/Model/Mesh/Creation/ModeModelMeshBevelEdges.o : Mode/Model/Mesh/Creation/ModeModelMeshBevelEdges.cpp
	$(CPP) -c Mode/Model/Mesh/Creation/ModeModelMeshBevelEdges.cpp -o $@ $(CXXFLAGS)

Mode/Model/Mesh/Creation/ModeModelMeshBrush.o : Mode/Model/Mesh/Creation/ModeModelMeshBrush.cpp
	$(CPP) -c Mode/Model/Mesh/Creation/ModeModelMeshBrush.cpp -o $@ $(CXXFLAGS)

Mode/Model/Mesh/Creation/ModeModelMeshCreateBall.o : Mode/Model/Mesh/Creation/ModeModelMeshCreateBall.cpp
	$(CPP) -c Mode/Model/Mesh/Creation/ModeModelMeshCreateBall.cpp -o $@ $(CXXFLAGS)

Mode/Model/Mesh/Creation/ModeModelMeshCreateCube.o : Mode/Model/Mesh/Creation/ModeModelMeshCreateCube.cpp
	$(CPP) -c Mode/Model/Mesh/Creation/ModeModelMeshCreateCube.cpp -o $@ $(CXXFLAGS)

Mode/Model/Mesh/Creation/ModeModelMeshCreateCylinder.o : Mode/Model/Mesh/Creation/ModeModelMeshCreateCylinder.cpp
	$(CPP) -c Mode/Model/Mesh/Creation/ModeModelMeshCreateCylinder.cpp -o $@ $(CXXFLAGS)

Mode/Model/Mesh/Creation/ModeModelMeshCreateCylinderSnake.o : Mode/Model/Mesh/Creation/ModeModelMeshCreateCylinderSnake.cpp
	$(CPP) -c Mode/Model/Mesh/Creation/ModeModelMeshCreateCylinderSnake.cpp -o $@ $(CXXFLAGS)

Mode/Model/Mesh/Creation/ModeModelMeshCreatePlane.o : Mode/Model/Mesh/Creation/ModeModelMeshCreatePlane.cpp
	$(CPP) -c Mode/Model/Mesh/Creation/ModeModelMeshCreatePlane.cpp -o $@ $(CXXFLAGS)

Mode/Model/Mesh/Creation/ModeModelMeshCreatePlatonic.o : Mode/Model/Mesh/Creation/ModeModelMeshCreatePlatonic.cpp
	$(CPP) -c Mode/Model/Mesh/Creation/ModeModelMeshCreatePlatonic.cpp -o $@ $(CXXFLAGS)

Mode/Model/Mesh/Creation/ModeModelMeshCreatePolygon.o : Mode/Model/Mesh/Creation/ModeModelMeshCreatePolygon.cpp
	$(CPP) -c Mode/Model/Mesh/Creation/ModeModelMeshCreatePolygon.cpp -o $@ $(CXXFLAGS)

Mode/Model/Mesh/Creation/ModeModelMeshCreateTorus.o : Mode/Model/Mesh/Creation/ModeModelMeshCreateTorus.cpp
	$(CPP) -c Mode/Model/Mesh/Creation/ModeModelMeshCreateTorus.cpp -o $@ $(CXXFLAGS)

Mode/Model/Mesh/Creation/ModeModelMeshCreateVertex.o : Mode/Model/Mesh/Creation/ModeModelMeshCreateVertex.cpp
	$(CPP) -c Mode/Model/Mesh/Creation/ModeModelMeshCreateVertex.cpp -o $@ $(CXXFLAGS)

Mode/Model/Mesh/Creation/ModeModelMeshDeformCylinder.o : Mode/Model/Mesh/Creation/ModeModelMeshDeformCylinder.cpp
	$(CPP) -c Mode/Model/Mesh/Creation/ModeModelMeshDeformCylinder.cpp -o $@ $(CXXFLAGS)

Mode/Model/Mesh/Creation/ModeModelMeshDeformFunction.o : Mode/Model/Mesh/Creation/ModeModelMeshDeformFunction.cpp
	$(CPP) -c Mode/Model/Mesh/Creation/ModeModelMeshDeformFunction.cpp -o $@ $(CXXFLAGS)

Mode/Model/Mesh/Creation/ModeModelMeshExtrudePolygons.o : Mode/Model/Mesh/Creation/ModeModelMeshExtrudePolygons.cpp
	$(CPP) -c Mode/Model/Mesh/Creation/ModeModelMeshExtrudePolygons.cpp -o $@ $(CXXFLAGS)

Mode/Model/Mesh/Creation/ModeModelMeshPaste.o : Mode/Model/Mesh/Creation/ModeModelMeshPaste.cpp
	$(CPP) -c Mode/Model/Mesh/Creation/ModeModelMeshPaste.cpp -o $@ $(CXXFLAGS)

Mode/Model/Mesh/Creation/ModeModelMeshSplitPolygon.o : Mode/Model/Mesh/Creation/ModeModelMeshSplitPolygon.cpp
	$(CPP) -c Mode/Model/Mesh/Creation/ModeModelMeshSplitPolygon.cpp -o $@ $(CXXFLAGS)

Mode/Model/Mesh/ModeModelMesh.o : Mode/Model/Mesh/ModeModelMesh.cpp
	$(CPP) -c Mode/Model/Mesh/ModeModelMesh.cpp -o $@ $(CXXFLAGS)

Mode/Model/Mesh/ModeModelMeshTexture.o : Mode/Model/Mesh/ModeModelMeshTexture.cpp
	$(CPP) -c Mode/Model/Mesh/ModeModelMeshTexture.cpp -o $@ $(CXXFLAGS)

Mode/Model/Mesh/Selection/MeshSelectionMode.o : Mode/Model/Mesh/Selection/MeshSelectionMode.cpp
	$(CPP) -c Mode/Model/Mesh/Selection/MeshSelectionMode.cpp -o $@ $(CXXFLAGS)

Mode/Model/Mesh/Selection/MeshSelectionModeEdge.o : Mode/Model/Mesh/Selection/MeshSelectionModeEdge.cpp
	$(CPP) -c Mode/Model/Mesh/Selection/MeshSelectionModeEdge.cpp -o $@ $(CXXFLAGS)

Mode/Model/Mesh/Selection/MeshSelectionModePolygon.o : Mode/Model/Mesh/Selection/MeshSelectionModePolygon.cpp
	$(CPP) -c Mode/Model/Mesh/Selection/MeshSelectionModePolygon.cpp -o $@ $(CXXFLAGS)

Mode/Model/Mesh/Selection/MeshSelectionModeSurface.o : Mode/Model/Mesh/Selection/MeshSelectionModeSurface.cpp
	$(CPP) -c Mode/Model/Mesh/Selection/MeshSelectionModeSurface.cpp -o $@ $(CXXFLAGS)

Mode/Model/Mesh/Selection/MeshSelectionModeVertex.o : Mode/Model/Mesh/Selection/MeshSelectionModeVertex.cpp
	$(CPP) -c Mode/Model/Mesh/Selection/MeshSelectionModeVertex.cpp -o $@ $(CXXFLAGS)

Mode/Model/ModeModel.o : Mode/Model/ModeModel.cpp
	$(CPP) -c Mode/Model/ModeModel.cpp -o $@ $(CXXFLAGS)

Mode/Model/Skeleton/Creation/ModeModelSkeletonAttachVertices.o : Mode/Model/Skeleton/Creation/ModeModelSkeletonAttachVertices.cpp
	$(CPP) -c Mode/Model/Skeleton/Creation/ModeModelSkeletonAttachVertices.cpp -o $@ $(CXXFLAGS)

Mode/Model/Skeleton/Creation/ModeModelSkeletonCreateBone.o : Mode/Model/Skeleton/Creation/ModeModelSkeletonCreateBone.cpp
	$(CPP) -c Mode/Model/Skeleton/Creation/ModeModelSkeletonCreateBone.cpp -o $@ $(CXXFLAGS)

Mode/Model/Skeleton/ModeModelSkeleton.o : Mode/Model/Skeleton/ModeModelSkeleton.cpp
	$(CPP) -c Mode/Model/Skeleton/ModeModelSkeleton.cpp -o $@ $(CXXFLAGS)

Mode/ModeNone.o : Mode/ModeNone.cpp
	$(CPP) -c Mode/ModeNone.cpp -o $@ $(CXXFLAGS)

Mode/Welcome/ModeWelcome.o : Mode/Welcome/ModeWelcome.cpp
	$(CPP) -c Mode/Welcome/ModeWelcome.cpp -o $@ $(CXXFLAGS)

Mode/World/Camera/Creation/ModeWorldCameraCreatePoint.o : Mode/World/Camera/Creation/ModeWorldCameraCreatePoint.cpp
	$(CPP) -c Mode/World/Camera/Creation/ModeWorldCameraCreatePoint.cpp -o $@ $(CXXFLAGS)

Mode/World/Camera/ModeWorldCamera.o : Mode/World/Camera/ModeWorldCamera.cpp
	$(CPP) -c Mode/World/Camera/ModeWorldCamera.cpp -o $@ $(CXXFLAGS)

Mode/World/Creation/ModeWorldCreateObject.o : Mode/World/Creation/ModeWorldCreateObject.cpp
	$(CPP) -c Mode/World/Creation/ModeWorldCreateObject.cpp -o $@ $(CXXFLAGS)

Mode/World/Creation/ModeWorldCreateTerrain.o : Mode/World/Creation/ModeWorldCreateTerrain.cpp
	$(CPP) -c Mode/World/Creation/ModeWorldCreateTerrain.cpp -o $@ $(CXXFLAGS)

Mode/World/Dialog/CameraDialog.o : Mode/World/Dialog/CameraDialog.cpp
	$(CPP) -c Mode/World/Dialog/CameraDialog.cpp -o $@ $(CXXFLAGS)

Mode/World/Dialog/LightmapDialog.o : Mode/World/Dialog/LightmapDialog.cpp
	$(CPP) -c Mode/World/Dialog/LightmapDialog.cpp -o $@ $(CXXFLAGS)

Mode/World/Dialog/ObjectPropertiesDialog.o : Mode/World/Dialog/ObjectPropertiesDialog.cpp
	$(CPP) -c Mode/World/Dialog/ObjectPropertiesDialog.cpp -o $@ $(CXXFLAGS)

Mode/World/Dialog/SelectionPropertiesDialog.o : Mode/World/Dialog/SelectionPropertiesDialog.cpp
	$(CPP) -c Mode/World/Dialog/SelectionPropertiesDialog.cpp -o $@ $(CXXFLAGS)

Mode/World/Dialog/TerrainCreationDialog.o : Mode/World/Dialog/TerrainCreationDialog.cpp
	$(CPP) -c Mode/World/Dialog/TerrainCreationDialog.cpp -o $@ $(CXXFLAGS)

Mode/World/Dialog/TerrainHeightmapDialog.o : Mode/World/Dialog/TerrainHeightmapDialog.cpp
	$(CPP) -c Mode/World/Dialog/TerrainHeightmapDialog.cpp -o $@ $(CXXFLAGS)

Mode/World/Dialog/TerrainPropertiesDialog.o : Mode/World/Dialog/TerrainPropertiesDialog.cpp
	$(CPP) -c Mode/World/Dialog/TerrainPropertiesDialog.cpp -o $@ $(CXXFLAGS)

Mode/World/Dialog/WorldPropertiesDialog.o : Mode/World/Dialog/WorldPropertiesDialog.cpp
	$(CPP) -c Mode/World/Dialog/WorldPropertiesDialog.cpp -o $@ $(CXXFLAGS)

Mode/World/ModeWorld.o : Mode/World/ModeWorld.cpp
	$(CPP) -c Mode/World/ModeWorld.cpp -o $@ $(CXXFLAGS)

Mode/World/Terrain/ModeWorldEditTerrain.o : Mode/World/Terrain/ModeWorldEditTerrain.cpp
	$(CPP) -c Mode/World/Terrain/ModeWorldEditTerrain.cpp -o $@ $(CXXFLAGS)

MultiView/ActionController.o : MultiView/ActionController.cpp
	$(CPP) -c MultiView/ActionController.cpp -o $@ $(CXXFLAGS)

MultiView/CameraController.o : MultiView/CameraController.cpp
	$(CPP) -c MultiView/CameraController.cpp -o $@ $(CXXFLAGS)

MultiView/MultiView.o : MultiView/MultiView.cpp
	$(CPP) -c MultiView/MultiView.cpp -o $@ $(CXXFLAGS)

MultiView/SingleData.o : MultiView/SingleData.cpp
	$(CPP) -c MultiView/SingleData.cpp -o $@ $(CXXFLAGS)

MultiView/Window.o : MultiView/Window.cpp
	$(CPP) -c MultiView/Window.cpp -o $@ $(CXXFLAGS)

Stuff/Observable.o : Stuff/Observable.cpp
	$(CPP) -c Stuff/Observable.cpp -o $@ $(CXXFLAGS)

Stuff/Observer.o : Stuff/Observer.cpp
	$(CPP) -c Stuff/Observer.cpp -o $@ $(CXXFLAGS)

Stuff/PluginManager.o : Stuff/PluginManager.cpp
	$(CPP) -c Stuff/PluginManager.cpp -o $@ $(CXXFLAGS)

Stuff/Progress.o : Stuff/Progress.cpp
	$(CPP) -c Stuff/Progress.cpp -o $@ $(CXXFLAGS)

x/camera.o : x/camera.cpp
	$(CPP) -c x/camera.cpp -o $@ $(CXXFLAGS)

x/font.o : x/font.cpp
	$(CPP) -c x/font.cpp -o $@ $(CXXFLAGS)

x/material.o : x/material.cpp
	$(CPP) -c x/material.cpp -o $@ $(CXXFLAGS)

x/model.o : x/model.cpp
	$(CPP) -c x/model.cpp -o $@ $(CXXFLAGS)

x/model_manager.o : x/model_manager.cpp
	$(CPP) -c x/model_manager.cpp -o $@ $(CXXFLAGS)

x/object.o : x/object.cpp
	$(CPP) -c x/object.cpp -o $@ $(CXXFLAGS)

x/terrain.o : x/terrain.cpp
	$(CPP) -c x/terrain.cpp -o $@ $(CXXFLAGS)

x/world.o : x/world.cpp
	$(CPP) -c x/world.cpp -o $@ $(CXXFLAGS)



clean :
	rm -f $(BIN)
	find . -name "*.o" -exec rm -f {} \;
	find . -name "*.a" -exec rm -f {} \;


