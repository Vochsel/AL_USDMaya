//
// Copyright 2018 Animal Logic
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.//
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "AL/usdmaya/utils/DiffPrimVar.h"
#include "AL/usdmaya/nodes/ProxyShape.h"
#include "maya/MFnMesh.h"
#include "maya/MSelectionList.h"
#include "maya/MFileIO.h"
#include "maya/MFnDagNode.h"
#include "maya/MGlobal.h"
#include "maya/MStringArray.h"
#include "maya/MPointArray.h"
#include <gtest/gtest.h>

using namespace AL::maya;
using namespace AL::usdmaya;

TEST(DiffPrimVar, diffGeomVerts)
{
  MFileIO::newFile(true);
  MStringArray result;
  ASSERT_TRUE(MGlobal::executeCommand("polySphere  -r 1 -sx 20 -sy 20 -ax 0 1 0 -cuv 2 -ch 1", result) == MS::kSuccess);

  const char* const exportCommand =
  "file -force -options \"Dynamic_Attributes=0;Meshes=1;Mesh_Normals=1;Nurbs_Curves=1;Duplicate_Instances=1;Use_Animal_Schema=0;Merge_Transforms=1;Animation=0;"
  "Use_Timeline_Range=0;Frame_Min=1;Frame_Max=50;Filter_Sample=0;\" -typ \"AL usdmaya export\" -pr -ea \"/tmp/AL_USDMayaTests_diffPrimVarVerts.usda\";";

  ASSERT_TRUE(MGlobal::executeCommand(exportCommand) == MS::kSuccess);
  ASSERT_TRUE(result.length() == 2);

  MSelectionList sl;
  EXPECT_TRUE(sl.add("pSphereShape1")  == MS::kSuccess);

  MObject obj;
  sl.getDependNode(0, obj);
  MStatus status;
  MFnMesh fn(obj, &status);

  {
    MFnDagNode fnd;
    MObject xform = fnd.create("transform");
    MObject shape = fnd.create("AL_usdmaya_ProxyShape", xform);

    AL::usdmaya::nodes::ProxyShape* proxy = (AL::usdmaya::nodes::ProxyShape*)fnd.userNode();

    // force the stage to load
    proxy->filePathPlug().setString("/tmp/AL_USDMayaTests_diffPrimVarVerts.usda");

    auto stage = proxy->getUsdStage();
    MString path = MString("/") + result[0];

    SdfPath primPath(path.asChar());
    UsdPrim geomPrim = stage->GetPrimAtPath(primPath);
    UsdGeomMesh geom(geomPrim);

    // hopefully nothing will have changed here
    uint32_t result = AL::usdmaya::utils::diffGeom(geom, fn, UsdTimeCode::Default(), AL::usdmaya::utils::kAllComponents);
    EXPECT_EQ(0, result);

    MPoint p, pm;
    fn.getPoint(4, p);
    pm = p;
    pm.x += 0.1f;
    fn.setPoint(4, pm);

    // hopefully nothing will have changed here
    result = AL::usdmaya::utils::diffGeom(geom, fn, UsdTimeCode::Default(), AL::usdmaya::utils::kAllComponents);
    EXPECT_EQ(AL::usdmaya::utils::kPoints | AL::usdmaya::utils::kNormals, result);

    fn.setPoint(4, p);
    result = AL::usdmaya::utils::diffGeom(geom, fn, UsdTimeCode::Default(), AL::usdmaya::utils::kAllComponents);
  }
}

TEST(DiffPrimVar, diffGeomNormals)
{
  MFileIO::newFile(true);
  MStringArray result;
  ASSERT_TRUE(MGlobal::executeCommand("polySphere  -r 1 -sx 20 -sy 20 -ax 0 1 0 -cuv 2 -ch 1", result) == MS::kSuccess);

  const char* const exportCommand =
  "file -force -options \"Dynamic_Attributes=0;Meshes=1;Mesh_Normals=1;Nurbs_Curves=1;Duplicate_Instances=1;Use_Animal_Schema=0;Merge_Transforms=1;Animation=0;"
  "Use_Timeline_Range=0;Frame_Min=1;Frame_Max=50;Filter_Sample=0;\" -typ \"AL usdmaya export\" -pr -ea \"/tmp/AL_USDMayaTests_diffPrimVarNormals.usda\";";

  ASSERT_TRUE(MGlobal::executeCommand(exportCommand) == MS::kSuccess);
  ASSERT_TRUE(result.length() == 2);

  MSelectionList sl;
  EXPECT_TRUE(sl.add("pSphereShape1")  == MS::kSuccess);

  MObject obj;
  sl.getDependNode(0, obj);
  MStatus status;
  MFnMesh fn(obj, &status);

  {
    MFnDagNode fnd;
    MObject xform = fnd.create("transform");
    MObject shape = fnd.create("AL_usdmaya_ProxyShape", xform);

    AL::usdmaya::nodes::ProxyShape* proxy = (AL::usdmaya::nodes::ProxyShape*)fnd.userNode();

    // force the stage to load
    proxy->filePathPlug().setString("/tmp/AL_USDMayaTests_diffPrimVarNormals.usda");

    auto stage = proxy->getUsdStage();
    MString path = MString("/") + result[0];

    SdfPath primPath(path.asChar());
    UsdPrim geomPrim = stage->GetPrimAtPath(primPath);
    UsdGeomMesh geom(geomPrim);

    // hopefully nothing will have changed here
    uint32_t result = AL::usdmaya::utils::diffGeom(geom, fn, UsdTimeCode::Default(), AL::usdmaya::utils::kAllComponents);
    EXPECT_EQ(0, result);

    MIntArray vertexList;
    fn.getPolygonVertices(2, vertexList);

    MVector n, nm;
    fn.getFaceVertexNormal(2, vertexList[0], n);
    nm = n;
    nm.x += 0.1f;
    nm.normalize();
    fn.setFaceVertexNormal(nm, 2, vertexList[0]);

    // hopefully nothing will have changed here
    result = AL::usdmaya::utils::diffGeom(geom, fn, UsdTimeCode::Default(), AL::usdmaya::utils::kAllComponents);
    EXPECT_EQ(AL::usdmaya::utils::kNormals, result);
  }
}

//uint32_t diffFaceVertices(UsdGeomMesh& geom, MFnMesh& mesh, UsdTimeCode timeCode, uint32_t exportMask = kAllComponents)
TEST(DiffPrimVar, diffFaceVertices)
{
  MFileIO::newFile(true);
  MStringArray result;
  ASSERT_TRUE(MGlobal::executeCommand("polySphere  -r 1 -sx 20 -sy 20 -ax 0 1 0 -cuv 2 -ch 1", result) == MS::kSuccess);

  const char* const exportCommand =
  "file -force -options \"Dynamic_Attributes=0;Meshes=1;Mesh_Normals=1;Nurbs_Curves=1;Duplicate_Instances=1;Use_Animal_Schema=0;Merge_Transforms=1;Animation=0;"
  "Use_Timeline_Range=0;Frame_Min=1;Frame_Max=50;Filter_Sample=0;\" -typ \"AL usdmaya export\" -pr -ea \"/tmp/AL_USDMayaTests_diffFaceVertices.usda\";";

  ASSERT_TRUE(MGlobal::executeCommand(exportCommand) == MS::kSuccess);
  ASSERT_TRUE(result.length() == 2);

  MSelectionList sl;
  EXPECT_TRUE(sl.add("pSphereShape1")  == MS::kSuccess);

  MObject obj;
  sl.getDependNode(0, obj);
  MStatus status;
  MFnMesh fn(obj, &status);

  sl.clear();

  {
    MFnDagNode fnd;
    MObject xform = fnd.create("transform");
    MObject shape = fnd.create("AL_usdmaya_ProxyShape", xform);

    AL::usdmaya::nodes::ProxyShape* proxy = (AL::usdmaya::nodes::ProxyShape*)fnd.userNode();

    // force the stage to load
    proxy->filePathPlug().setString("/tmp/AL_USDMayaTests_diffFaceVertices.usda");

    auto stage = proxy->getUsdStage();
    MString path = MString("/") + result[0];

    SdfPath primPath(path.asChar());
    UsdPrim geomPrim = stage->GetPrimAtPath(primPath);
    UsdGeomMesh geom(geomPrim);

    // hopefully nothing will have changed here
    uint32_t result = AL::usdmaya::utils::diffFaceVertices(geom, fn, UsdTimeCode::Default(), AL::usdmaya::utils::kAllComponents);
    EXPECT_EQ(0, result);

    // a command that will extrude the final triangle, and delete the 4 new faces.
    // Result should be the same number of poly counts, but the face vertices will have changed
    const char* removeFacesCommand =
    "undoInfo -st 1;"
    "polyExtrudeFacet -constructionHistory 1 -keepFacesTogether 1 -pvx 0.6465707123 -pvy 0.3815037459 -pvz 0.6465707719 -divisions 1 -twist 0 -taper 1 -off 0 -thickness 0 -smoothingAngle 30 pSphere1.f[399];\n"
    "setAttr \"polyExtrudeFace1.localTranslate\" -type double3 0 0 0.078999;\n"
    "select -r pSphere1.f[400] pSphere1.f[401] pSphere1.f[402];\n"
    "doDelete;\n";
    ASSERT_TRUE(MGlobal::executeCommand(removeFacesCommand) == MS::kSuccess);

    EXPECT_TRUE(sl.add("pSphereShape1")  == MS::kSuccess);
    sl.getDependNode(0, obj);
    fn.setObject(obj);

    // hopefully nothing will have changed here
    result = AL::usdmaya::utils::diffFaceVertices(geom, fn, UsdTimeCode::Default(), AL::usdmaya::utils::kAllComponents);
    EXPECT_EQ(AL::usdmaya::utils::kFaceVertexIndices, result);

    MGlobal::executeCommand("undo");

    const char* removeFacesCommand2 =
    "polyExtrudeFacet -constructionHistory 1 -keepFacesTogether 1 -pvx 0.6465707123 -pvy 0.3815037459 -pvz 0.6465707719 -divisions 1 -twist 0 -taper 1 -off 0 -thickness 0 -smoothingAngle 30 pSphere1.f[399];\n"
    "setAttr \"polyExtrudeFace1.localTranslate\" -type double3 0 0 0.078999;\n"
    "select -r pSphere1.f[399] pSphere1.f[401] pSphere1.f[402];\n"
    "doDelete;\n";
    ASSERT_TRUE(MGlobal::executeCommand(removeFacesCommand) == MS::kSuccess);

    sl.clear();
    EXPECT_TRUE(sl.add("pSphereShape1")  == MS::kSuccess);

    sl.getDependNode(0, obj);
    fn.setObject(obj);

    result = AL::usdmaya::utils::diffFaceVertices(geom, fn, UsdTimeCode::Default(), AL::usdmaya::utils::kAllComponents);
    EXPECT_EQ(AL::usdmaya::utils::kFaceVertexCounts | AL::usdmaya::utils::kFaceVertexIndices, result);
  }
}

// test the holes set via the invisible faces param
TEST(DiffPrimVar, diffHoles1)
{
  MFileIO::newFile(true);
  MStringArray result;

  ASSERT_TRUE(MGlobal::executeCommand("polyCube -w 1 -h 1 -d 1 -sx 1 -sy 1 -sz 1 -ax 0 1 0 -cuv 2 -ch 1", result) == MS::kSuccess);
  ASSERT_TRUE(result.length() == 2);

  ASSERT_TRUE(MGlobal::executeCommand("delete -ch pCubeShape1") == MS::kSuccess);

  MSelectionList sl;
  EXPECT_TRUE(sl.add("pCubeShape1")  == MS::kSuccess);

  MObject obj;
  MStatus status;
  MFnMesh fn;
  {
    sl.getDependNode(0, obj);
    status = fn.setObject(obj);

    MUintArray invisbleFaces;
    invisbleFaces.append(2);
    ASSERT_TRUE(fn.setInvisibleFaces(invisbleFaces) == MS::kSuccess);
  }

  const char* const exportCommand =
  "file -force -options \"Dynamic_Attributes=0;Meshes=1;Mesh_Normals=1;Nurbs_Curves=1;Duplicate_Instances=1;Use_Animal_Schema=0;Merge_Transforms=1;Animation=0;"
  "Use_Timeline_Range=0;Frame_Min=1;Frame_Max=50;Filter_Sample=0;\" -typ \"AL usdmaya export\" -pr -ea \"/tmp/AL_USDMayaTests_diffHoles1.usda\";";
  ASSERT_TRUE(MGlobal::executeCommand(exportCommand) == MS::kSuccess);

  {
    MFnDagNode fnd;
    MObject xform = fnd.create("transform");
    MObject shape = fnd.create("AL_usdmaya_ProxyShape", xform);

    AL::usdmaya::nodes::ProxyShape* proxy = (AL::usdmaya::nodes::ProxyShape*)fnd.userNode();

    // force the stage to load
    proxy->filePathPlug().setString("/tmp/AL_USDMayaTests_diffHoles1.usda");

    auto stage = proxy->getUsdStage();
    MString path = MString("/") + result[0];

    SdfPath primPath(path.asChar());
    UsdPrim geomPrim = stage->GetPrimAtPath(primPath);
    UsdGeomMesh geom(geomPrim);

    EXPECT_EQ(0, AL::usdmaya::utils::diffFaceVertices(geom, fn, UsdTimeCode::Default(), AL::usdmaya::utils::kAllComponents));

    {
      MUintArray invisbleFaces;
      invisbleFaces.append(3);
      ASSERT_TRUE(fn.setInvisibleFaces(invisbleFaces) == MS::kSuccess);
    }

    EXPECT_EQ(AL::usdmaya::utils::kHoleIndices, AL::usdmaya::utils::diffFaceVertices(geom, fn, UsdTimeCode::Default(), AL::usdmaya::utils::kAllComponents));
  }
  sl.clear();
}

// test the holes set via the addHoles approach of defining holes via the magical second approach
TEST(DiffPrimVar, diffCreaseEdges)
{
  MFileIO::newFile(true);
  MStringArray result;

  ASSERT_TRUE(MGlobal::executeCommand("polyCube -w 1 -h 1 -d 1 -sx 1 -sy 1 -sz 1 -ax 0 1 0 -cuv 2 -ch 1", result) == MS::kSuccess);
  ASSERT_TRUE(result.length() == 2);

  ASSERT_TRUE(MGlobal::executeCommand("polyCrease -ch true -value 0.96 -vertexValue 0.96 pCube1.e[2]") == MS::kSuccess);

  const char* const exportCommand =
  "file -force -options \"Dynamic_Attributes=0;Meshes=1;Mesh_Normals=1;Nurbs_Curves=1;Duplicate_Instances=1;Use_Animal_Schema=0;Merge_Transforms=1;Animation=0;"
  "Use_Timeline_Range=0;Frame_Min=1;Frame_Max=50;Filter_Sample=0;\" -typ \"AL usdmaya export\" -pr -ea \"/tmp/AL_USDMayaTests_diffCreaseEdges.usda\";";

  ASSERT_TRUE(MGlobal::executeCommand(exportCommand) == MS::kSuccess);


  MSelectionList sl;
  EXPECT_TRUE(sl.add("pCubeShape1")  == MS::kSuccess);

  MObject obj;
  MStatus status;
  MFnMesh fn;

  sl.getDependNode(0, obj);
  status = fn.setObject(obj);
  EXPECT_TRUE(status == MS::kSuccess);

  {
    MFnDagNode fnd;
    MObject xform = fnd.create("transform");
    MObject shape = fnd.create("AL_usdmaya_ProxyShape", xform);

    AL::usdmaya::nodes::ProxyShape* proxy = (AL::usdmaya::nodes::ProxyShape*)fnd.userNode();

    // force the stage to load
    proxy->filePathPlug().setString("/tmp/AL_USDMayaTests_diffCreaseEdges.usda");

    auto stage = proxy->getUsdStage();
    MString path = MString("/") + result[0];

    SdfPath primPath(path.asChar());
    UsdPrim geomPrim = stage->GetPrimAtPath(primPath);
    UsdGeomMesh geom(geomPrim);

    EXPECT_EQ(0, AL::usdmaya::utils::diffFaceVertices(geom, fn, UsdTimeCode::Default(), AL::usdmaya::utils::kAllComponents));

    ASSERT_TRUE(MGlobal::executeCommand("delete pCube1"));


    ASSERT_TRUE(MGlobal::executeCommand("polyCube -w 1 -h 1 -d 1 -sx 1 -sy 1 -sz 1 -ax 0 1 0 -cuv 2 -ch 1", result) == MS::kSuccess);
    ASSERT_TRUE(result.length() == 2);

    ASSERT_TRUE(MGlobal::executeCommand("polyCrease -ch true -value 0.96 -vertexValue 0.96 pCube1.e[3]") == MS::kSuccess);

    sl.clear();
    EXPECT_TRUE(sl.add("pCubeShape1")  == MS::kSuccess);

    sl.getDependNode(0, obj);
    status = fn.setObject(obj);
    EXPECT_TRUE(status == MS::kSuccess);

    EXPECT_EQ(AL::usdmaya::utils::kCreaseIndices, AL::usdmaya::utils::diffFaceVertices(geom, fn, UsdTimeCode::Default(), AL::usdmaya::utils::kAllComponents));

    ASSERT_TRUE(MGlobal::executeCommand("delete pCube1"));

    ASSERT_TRUE(MGlobal::executeCommand("polyCube -w 1 -h 1 -d 1 -sx 1 -sy 1 -sz 1 -ax 0 1 0 -cuv 2 -ch 1", result) == MS::kSuccess);
    ASSERT_TRUE(result.length() == 2);

    ASSERT_TRUE(MGlobal::executeCommand("polyCrease -ch true -value 0.22 -vertexValue 0.11 pCube1.e[2]") == MS::kSuccess);

    sl.clear();
    EXPECT_TRUE(sl.add("pCubeShape1")  == MS::kSuccess);

    sl.getDependNode(0, obj);
    status = fn.setObject(obj);
    EXPECT_TRUE(status == MS::kSuccess);

    EXPECT_EQ(AL::usdmaya::utils::kCreaseWeights, AL::usdmaya::utils::diffFaceVertices(geom, fn, UsdTimeCode::Default(), AL::usdmaya::utils::kAllComponents));
  }
}


// test the holes set via the addHoles approach of defining holes via the magical second approach
TEST(DiffPrimVar, diffCreaseVertices)
{
  MFileIO::newFile(true);
  MStringArray result;

  ASSERT_TRUE(MGlobal::executeCommand("polyCube -w 1 -h 1 -d 1 -sx 1 -sy 1 -sz 1 -ax 0 1 0 -cuv 2 -ch 1", result) == MS::kSuccess);
  ASSERT_TRUE(result.length() == 2);

  ASSERT_TRUE(MGlobal::executeCommand("polyCrease -ch true -value 0.96 -vertexValue 0.96 pCube1.vtx[2]") == MS::kSuccess);

  const char* const exportCommand =
  "file -force -options \"Dynamic_Attributes=0;Meshes=1;Mesh_Normals=1;Nurbs_Curves=1;Duplicate_Instances=1;Use_Animal_Schema=0;Merge_Transforms=1;Animation=0;"
  "Use_Timeline_Range=0;Frame_Min=1;Frame_Max=50;Filter_Sample=0;\" -typ \"AL usdmaya export\" -pr -ea \"/tmp/aaAL_USDMayaTests_diffCreaseVertices.usda\";";

  ASSERT_TRUE(MGlobal::executeCommand(exportCommand) == MS::kSuccess);

  MSelectionList sl;
  EXPECT_TRUE(sl.add("pCubeShape1")  == MS::kSuccess);

  MObject obj;
  MStatus status;
  MFnMesh fn;

  sl.getDependNode(0, obj);
  status = fn.setObject(obj);
  EXPECT_TRUE(status == MS::kSuccess);

  {
    MFnDagNode fnd;
    MObject xform = fnd.create("transform");
    MObject shape = fnd.create("AL_usdmaya_ProxyShape", xform);

    AL::usdmaya::nodes::ProxyShape* proxy = (AL::usdmaya::nodes::ProxyShape*)fnd.userNode();

    // force the stage to load
    proxy->filePathPlug().setString("/tmp/aaAL_USDMayaTests_diffCreaseVertices.usda");

    auto stage = proxy->getUsdStage();
    MString path = MString("/") + result[0];

    SdfPath primPath(path.asChar());
    UsdPrim geomPrim = stage->GetPrimAtPath(primPath);
    UsdGeomMesh geom(geomPrim);

    EXPECT_EQ(0, AL::usdmaya::utils::diffFaceVertices(geom, fn, UsdTimeCode::Default(), AL::usdmaya::utils::kAllComponents));

    ASSERT_TRUE(MGlobal::executeCommand("delete pCubeShape1"));
    ASSERT_TRUE(MGlobal::executeCommand("delete pCube1"));

    ASSERT_TRUE(MGlobal::executeCommand("polyCube -w 1 -h 1 -d 1 -sx 1 -sy 1 -sz 1 -ax 0 1 0 -cuv 2 -ch 1", result) == MS::kSuccess);
    ASSERT_TRUE(result.length() == 2);

    ASSERT_TRUE(MGlobal::executeCommand("polyCrease -ch true -value 0.96 -vertexValue 0.96 pCube1.vtx[3]") == MS::kSuccess);

    MFileIO::saveAs("/scratch/wat.mb");
    sl.clear();
    EXPECT_TRUE(sl.add("pCubeShape1")  == MS::kSuccess);

    sl.getDependNode(0, obj);
    status = fn.setObject(obj);
    EXPECT_TRUE(status == MS::kSuccess);

    EXPECT_EQ(AL::usdmaya::utils::kCornerIndices, AL::usdmaya::utils::diffFaceVertices(geom, fn, UsdTimeCode::Default(), AL::usdmaya::utils::kAllComponents));
    
    ASSERT_TRUE(MGlobal::executeCommand("delete pCube1"));

    ASSERT_TRUE(MGlobal::executeCommand("polyCube -w 1 -h 1 -d 1 -sx 1 -sy 1 -sz 1 -ax 0 1 0 -cuv 2 -ch 1", result) == MS::kSuccess);
    ASSERT_TRUE(result.length() == 2);

    ASSERT_TRUE(MGlobal::executeCommand("polyCrease -ch true -value 0.22 -vertexValue 0.22 pCube1.vtx[2]") == MS::kSuccess);

    sl.clear();
    EXPECT_TRUE(sl.add("pCubeShape1")  == MS::kSuccess);

    sl.getDependNode(0, obj);
    status = fn.setObject(obj);
    EXPECT_TRUE(status == MS::kSuccess);

    EXPECT_EQ(AL::usdmaya::utils::kCornerSharpness, AL::usdmaya::utils::diffFaceVertices(geom, fn, UsdTimeCode::Default(), AL::usdmaya::utils::kAllComponents));
  }
}

// test to see if the additional uv sets are handled.
TEST(DiffPrimVar, diffUvSetNames)
{
  MFileIO::newFile(true);
  MStringArray result;

  ASSERT_TRUE(MGlobal::executeCommand("polyCube -w 1 -h 1 -d 1 -sx 1 -sy 1 -sz 1 -ax 0 1 0 -cuv 2 -ch 0", result) == MS::kSuccess);
  ASSERT_TRUE(result.length() == 1);

  const char* const exportCommand =
  "file -force -options \"Dynamic_Attributes=0;Meshes=1;Mesh_Normals=1;Nurbs_Curves=1;Duplicate_Instances=1;Use_Animal_Schema=0;Merge_Transforms=1;Animation=0;"
  "Use_Timeline_Range=0;Frame_Min=1;Frame_Max=50;Filter_Sample=0;\" -typ \"AL usdmaya export\" -pr -ea \"/tmp/AL_USDMayaTests_diffUvSetNames.usda\";";

  ASSERT_TRUE(MGlobal::executeCommand(exportCommand) == MS::kSuccess);

  MFnDagNode fnd;
  MObject xform = fnd.create("transform");
  MObject shape = fnd.create("AL_usdmaya_ProxyShape", xform);

  AL::usdmaya::nodes::ProxyShape* proxy = (AL::usdmaya::nodes::ProxyShape*)fnd.userNode();

  // force the stage to load
  proxy->filePathPlug().setString("/tmp/AL_USDMayaTests_diffUvSetNames.usda");

  auto stage = proxy->getUsdStage();
  MString path = MString("/") + result[0];

  SdfPath primPath(path.asChar());
  UsdPrim geomPrim = stage->GetPrimAtPath(primPath);
  UsdGeomMesh geom(geomPrim);

  {
    MSelectionList sl;
    EXPECT_TRUE(sl.add("pCubeShape1") == MS::kSuccess);

    MObject obj;
    MStatus status;
    MFnMesh fn;

    sl.getDependNode(0, obj);
    status = fn.setObject(obj);
    EXPECT_TRUE(status == MS::kSuccess);

    AL::usdmaya::utils::PrimVarDiffReport r;
    MStringArray names = AL::usdmaya::utils::hasNewUvSet(geom, fn, r);
    EXPECT_EQ(0, names.length());
    EXPECT_EQ(0, r.size());

    fn.createUVSetWithName("newUvSet");

    r.clear();
    names = AL::usdmaya::utils::hasNewUvSet(geom, fn, r);
    EXPECT_EQ(0, r.size());
    ASSERT_EQ(1, names.length());
    EXPECT_EQ(MString("newUvSet"), names[0]);

    // extract the uv coords, modify them slightly, and pass back to maya
    MFloatArray us, vs;
    MString name("map1");
    fn.getUVs(us, vs, &name);
    vs[0] -= 0.1f;
    EXPECT_TRUE(fn.setUVs(us, vs, &name) == MS::kSuccess);

    r.clear();
    names = AL::usdmaya::utils::hasNewUvSet(geom, fn, r);
    ASSERT_EQ(1, r.size());

    {
      const AL::usdmaya::utils::PrimVarDiffEntry& pve = r[0];
      EXPECT_FALSE(pve.isColourSet());
      EXPECT_TRUE(pve.isUvSet());
      EXPECT_TRUE(pve.setName() == "map1");
      EXPECT_TRUE(pve.dataHasChanged());
      EXPECT_FALSE(pve.indicesHaveChanged());
    }

    vs[0] += 0.1f;
    EXPECT_TRUE(fn.setUVs(us, vs, &name) == MS::kSuccess);
    r.clear();
    names = AL::usdmaya::utils::hasNewUvSet(geom, fn, r);
    ASSERT_EQ(0, r.size());

    MIntArray uvCounts, mayaUvIndices;
    EXPECT_TRUE(fn.getAssignedUVs(uvCounts, mayaUvIndices, &name) == MS::kSuccess);
    int temp = mayaUvIndices[4];
    mayaUvIndices[4] = 0;

    EXPECT_TRUE(fn.assignUVs(uvCounts, mayaUvIndices, &name) == MS::kSuccess);

    r.clear();
    names = AL::usdmaya::utils::hasNewUvSet(geom, fn, r);
    ASSERT_EQ(1, r.size());

    {
      const AL::usdmaya::utils::PrimVarDiffEntry& pve = r[0];
      EXPECT_FALSE(pve.isColourSet());
      EXPECT_TRUE(pve.isUvSet());
      EXPECT_TRUE(pve.setName() == "map1");
      EXPECT_FALSE(pve.dataHasChanged());
      EXPECT_TRUE(pve.indicesHaveChanged());
    }
  }
}

// test to see if the additional uv sets are handled.
TEST(DiffPrimVar, diffColourSetNames)
{
  MFileIO::newFile(true);
  MStringArray result;

  ASSERT_TRUE(MGlobal::executeCommand("polyCube -w 1 -h 1 -d 1 -sx 1 -sy 1 -sz 1 -ax 0 1 0 -cuv 2 -ch 0", result) == MS::kSuccess);
  ASSERT_EQ(1, result.length());

  {
    MSelectionList sl;
    EXPECT_TRUE(sl.add("pCubeShape1") == MS::kSuccess);

    MObject obj;
    MStatus status;
    MFnMesh fn;

    sl.getDependNode(0, obj);
    status = fn.setObject(obj);
    EXPECT_TRUE(status == MS::kSuccess);

    MColorArray colours;
    MString setName = "firstSet";
    fn.createColorSetWithName(setName);
    colours.setLength(fn.numFaceVertices());
    for(int i = 0; i < colours.length(); ++i)
    {
      colours[i] = MColor(0, 0, 0, 1);
    }
    fn.setColors(colours, &setName);

    const char* const exportCommand =
    "file -force -options \"Dynamic_Attributes=0;Meshes=1;Mesh_Normals=1;Nurbs_Curves=1;Duplicate_Instances=1;Use_Animal_Schema=0;Merge_Transforms=1;Animation=0;"
    "Use_Timeline_Range=0;Frame_Min=1;Frame_Max=50;Filter_Sample=0;\" -typ \"AL usdmaya export\" -pr -ea \"/tmp/AL_USDMayaTests_diffColourSetNames.usda\";";

    ASSERT_TRUE(MGlobal::executeCommand(exportCommand) == MS::kSuccess);

    MFnDagNode fnd;
    MObject xform = fnd.create("transform");
    MObject shape = fnd.create("AL_usdmaya_ProxyShape", xform);

    AL::usdmaya::nodes::ProxyShape* proxy = (AL::usdmaya::nodes::ProxyShape*)fnd.userNode();

    // force the stage to load
    proxy->filePathPlug().setString("/tmp/AL_USDMayaTests_diffColourSetNames.usda");

    auto stage = proxy->getUsdStage();
    MString path = MString("/") + result[0];

    SdfPath primPath(path.asChar());
    UsdPrim geomPrim = stage->GetPrimAtPath(primPath);
    UsdGeomMesh geom(geomPrim);

    AL::usdmaya::utils::PrimVarDiffReport r;
    MStringArray names = AL::usdmaya::utils::hasNewColourSet(geom, fn, r);
    EXPECT_EQ(0, names.length());
    EXPECT_EQ(0, r.size());

    colours[colours.length() - 1].r = 0.1f;
    fn.setColors(colours, &setName);

    names = AL::usdmaya::utils::hasNewColourSet(geom, fn, r);
    EXPECT_EQ(0, names.length());
    ASSERT_EQ(1, r.size());

    {
      const AL::usdmaya::utils::PrimVarDiffEntry& pve = r[0];
      EXPECT_TRUE(pve.isColourSet());
      EXPECT_FALSE(pve.isUvSet());
      EXPECT_TRUE(pve.setName() == "firstSet");
      EXPECT_TRUE(pve.dataHasChanged());
      EXPECT_FALSE(pve.indicesHaveChanged());
    }
    colours[colours.length() - 1].r = 0.0f;
    fn.setColors(colours, &setName);

    r.clear();
    names = AL::usdmaya::utils::hasNewColourSet(geom, fn, r);
    EXPECT_EQ(0, names.length());
    ASSERT_EQ(0, r.size());

    fn.createColorSetWithName("newColorSet");

    names = AL::usdmaya::utils::hasNewColourSet(geom, fn, r);
    EXPECT_EQ(0, r.size());
    ASSERT_EQ(1, names.length());
    EXPECT_EQ(MString("newColorSet"), names[0]);
  }
}

/*
// test the holes set via the addHoles approach of defining holes via the magical second approach
TEST(DiffPrimVar, diffHoles2)
{
  MFileIO::newFile(true);
  MStringArray result;

  ASSERT_TRUE(MGlobal::executeCommand("polyCube -w 1 -h 1 -d 1 -sx 1 -sy 1 -sz 1 -ax 0 1 0 -cuv 2 -ch 1", result) == MS::kSuccess);
  ASSERT_TRUE(result.length() == 2);

  ASSERT_TRUE(MGlobal::executeCommand("delete -ch pCubeShape1") == MS::kSuccess);

  MSelectionList sl;
  EXPECT_TRUE(sl.add("pCubeShape1")  == MS::kSuccess);

  MObject obj;
  MStatus status;
  MFnMesh fn;
  {
    sl.getDependNode(0, obj);
    status = fn.setObject(obj);

    MPointArray points;
    points.append(MPoint(-0.2, 0.3, 0.5));
    points.append(MPoint(-0.3, 0.1, 0.5));
    points.append(MPoint( 0.1,-0.1, 0.5));
    MIntArray loopCounts;
    loopCounts.append(3);
    ASSERT_TRUE(fn.addHoles(0, points, loopCounts) == MS::kSuccess);

    MFileIO::saveAs("/tmp/aaShit.mb", 0, true);
  }

  const char* const exportCommand =
  "file -force -options \"Dynamic_Attributes=0;Meshes=1;Mesh_Normals=1;Nurbs_Curves=1;Duplicate_Instances=1;Use_Animal_Schema=0;Merge_Transforms=1;Animation=0;"
  "Use_Timeline_Range=0;Frame_Min=1;Frame_Max=50;Filter_Sample=0;\" -typ \"AL usdmaya export\" -pr -ea \"/tmp/aaAL_USDMaya_diffHoles.usda\";";

  ASSERT_TRUE(MGlobal::executeCommand(exportCommand) == MS::kSuccess);

  sl.clear();
  EXPECT_TRUE(sl.add("pCubeShape1")  == MS::kSuccess);

  sl.getDependNode(0, obj);
  fn.setObject(obj);

  {
    MIntArray mayaHoleIndices, mayaHoleVertexIndices;
    fn.getHoles(mayaHoleIndices, mayaHoleVertexIndices);

    std::cout << "mayaHoleIndices " << mayaHoleIndices.length() << std::endl;
    for(size_t i = 0; i < mayaHoleIndices.length(); ++i)
    {
      std::cout << mayaHoleIndices[i] << std::endl;
    }
    std::cout << "mayaHoleVertexIndices " << mayaHoleVertexIndices.length() << std::endl;
    for(size_t i = 0; i < mayaHoleVertexIndices.length(); ++i)
    {
      std::cout << mayaHoleVertexIndices[i] << std::endl;
    }

    MUintArray mayaInvisi = fn.getInvisibleFaces();
    std::cout << "mayaInvisibleFaces " << mayaInvisi.length() << std::endl;
    for(uint32_t i = 0; i < mayaInvisi.length(); ++i)
    {
      std::cout << mayaInvisi[i] << std::endl;
    }
  }

  {
    MFnDagNode fnd;
    MObject xform = fnd.create("transform");
    MObject shape = fnd.create("AL_usdmaya_ProxyShape", xform);

    AL::usdmaya::nodes::ProxyShape* proxy = (AL::usdmaya::nodes::ProxyShape*)fnd.userNode();

    // force the stage to load
    proxy->filePathPlug().setString("/tmp/aaAL_USDMaya_diffHoles.usda");

    auto stage = proxy->getUsdStage();
    MString path = MString("/") + result[0];

    SdfPath primPath(path.asChar());
    UsdPrim geomPrim = stage->GetPrimAtPath(primPath);
    UsdGeomMesh geom(geomPrim);

    EXPECT_EQ(0, AL::usdmaya::utils::diffFaceVertices(geom, fn, UsdTimeCode::Default(), AL::usdmaya::utils::kAllComponents));

    MObject parent = fn.parent(0);
    EXPECT_TRUE(MGlobal::deleteNode(parent) == MS::kSuccess);

    ASSERT_TRUE(MGlobal::executeCommand("polyCube -w 1 -h 1 -d 1 -sx 1 -sy 1 -sz 1 -ax 0 1 0 -cuv 2 -ch 1", result) == MS::kSuccess);
    ASSERT_TRUE(result.length() == 2);
    ASSERT_TRUE(MGlobal::executeCommand("delete -ch pCubeShape1") == MS::kSuccess);
    {
      sl.clear();
      EXPECT_TRUE(sl.add("pCubeShape1")  == MS::kSuccess);
      sl.getDependNode(0, obj);
      fn.setObject(obj);

      MPointArray points;
      points.append(MPoint(-0.2, 0.3,-0.5));
      points.append(MPoint(-0.3, 0.1,-0.5));
      points.append(MPoint( 0.1,-0.1,-0.5));
      MIntArray loopCounts;
      loopCounts.append(3);
      ASSERT_TRUE(fn.addHoles(2, points, loopCounts) == MS::kSuccess);

      sl.clear();
      EXPECT_TRUE(sl.add("pCubeShape1")  == MS::kSuccess);
      sl.getDependNode(0, obj);
      fn.setObject(obj);
    }

    EXPECT_EQ(AL::usdmaya::utils::kHoleIndices | AL::usdmaya::utils::kFaceVertexIndices, AL::usdmaya::utils::diffFaceVertices(geom, fn, UsdTimeCode::Default(), AL::usdmaya::utils::kAllComponents));

  }


  sl.clear();
}
*/

