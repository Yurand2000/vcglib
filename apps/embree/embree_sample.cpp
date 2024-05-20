/*

  g++ ./apps/embree/embree_sample.cpp -o  ./apps/embree/prova.o -I ./vcg -I ./ -I ./eigenlib -I /usr/local/include/embree4/ -I ./vcglib/ -I ./vcglib/eigenlib/ -I ./vcglib/wrap/ply/ -L /usr/local/lib/ -lembree4 -std=c++17 -fopenmp -O3
  ./apps/embree/prova.o ./apps/meshes/cube.off 64

  or with cmake from embree folder
  mkdir build
  cd build
  cmake ..
  make 
  ./embree_sample ../../meshes/bunny.off 64


*/
#include <iostream>

#include <vcg/complex/complex.h>
#include <vcg/complex/algorithms/create/platonic.h>


//import export
#include<wrap/io_trimesh/import.h>
#include<wrap/io_trimesh/export.h>
#include<wrap/io_trimesh/import_ply.h>
#include<wrap/io_trimesh/export_ply.h>
#include <wrap/io_trimesh/export_off.h>
#include <wrap/io_trimesh/import_off.h>

#include <time.h>
#include <vcg/math/gen_normal.h>
#include <vcg/complex/allocate.h>

//vcgLibForEmbree
#include<wrap/embree/EmbreeAdaptor.h>

class MyVertex; class MyEdge; class MyFace;
struct MyUsedTypes : public vcg::UsedTypes<vcg::Use<MyVertex>   ::AsVertexType,
    vcg::Use<MyEdge>     ::AsEdgeType,
    vcg::Use<MyFace>     ::AsFaceType> {};

class MyVertex : public vcg::Vertex< MyUsedTypes, vcg::vertex::Coord3f, vcg::vertex::Normal3f, vcg::vertex::BitFlags, vcg::vertex::VFAdj, vcg::vertex::Qualityf, vcg::vertex::Color4b> {};
class MyFace : public vcg::Face<   MyUsedTypes, vcg::face::FFAdj, vcg::face::VFAdj, vcg::face::Normal3f, vcg::face::VertexRef, vcg::face::BitFlags, vcg::face::Color4b, vcg::face::Qualityf> {};
class MyEdge : public vcg::Edge<MyUsedTypes, vcg::edge::VertexRef, vcg::edge::BitFlags> {};
class MyMesh : public vcg::tri::TriMesh< std::vector<MyVertex>, std::vector<MyFace>, std::vector<MyEdge>  > {};

using namespace vcg;
using namespace std;


int main( int argc, char **argv )
{
  cout << "start" << endl;
  MyMesh m;
  int ret = tri::io::ImporterOFF<MyMesh>::Open(m, argv[1]);
  if(ret!=tri::io::ImporterOFF<MyMesh>::NoError)
  {
    cout<<"Error reading file \n"<<endl;
    exit(0);
  }

  char *endptr;
  int nOfRays = 150; //strtof(argv[2], &endptr);
  if (argc > 2) {
      nOfRays = std::stoi(argv[2]);
  }

  
  MyMesh m2,m3,m4,m5,m6,m7;
  vcg::tri::Append<MyMesh,MyMesh>::MeshCopy(m2,m);
  vcg::tri::Append<MyMesh,MyMesh>::MeshCopy(m3,m);
  vcg::tri::Append<MyMesh,MyMesh>::MeshCopy(m4, m);
  vcg::tri::Append<MyMesh, MyMesh>::MeshCopy(m5, m);
  vcg::tri::Append<MyMesh,MyMesh>::MeshCopy(m6,m);
  vcg::tri::Append<MyMesh,MyMesh>::MeshCopy(m7,m);

  EmbreeAdaptor<MyMesh> adaptor = EmbreeAdaptor<MyMesh>(m);
  adaptor.computeAmbientOcclusion(m,nOfRays);
  tri::UpdateQuality<MyMesh>::VertexFromFace(m);
  tri::UpdateColor<MyMesh>::PerVertexQualityGray(m);
  tri::UpdateNormal<MyMesh>::NormalizePerVertex(m);
  tri::io::ExporterOFF<MyMesh>::Save(m,"testAO.off",tri::io::Mask::IOM_VERTCOLOR);
 
  cout << "Done AO" << endl;

   
  std::vector<Point3f> unifDirVec;
  std::vector<Point3f> ndir;
	GenNormal<float>::Fibonacci(nOfRays,unifDirVec);
    Point3f dir(0, 1, 0);

    for (int g = 0; g < nOfRays; g++) {
        if (unifDirVec.at(g) >= dir) {
            ndir.push_back(unifDirVec.at(g));
        }
    }
  adaptor = EmbreeAdaptor<MyMesh>(m2);
  adaptor.computeAmbientOcclusion(m2,ndir);
  tri::UpdateQuality<MyMesh>::VertexFromFace(m2);
  tri::UpdateColor<MyMesh>::PerVertexQualityGray(m2);
  tri::io::ExporterOFF<MyMesh>::Save(m2,"testAODir.off",tri::io::Mask::IOM_VERTCOLOR);

  cout << "Done AO Directioned" << endl;
  
  EmbreeAdaptor<MyMesh> adaptor2 = EmbreeAdaptor<MyMesh>(m4);
  adaptor2.computeSDF(m4,nOfRays,90);
  tri::UpdateQuality<MyMesh>::VertexFromFace(m4);
  tri::UpdateColor<MyMesh>::PerVertexQualityRamp(m4);
  tri::io::ExporterOFF<MyMesh>::Save(m4,"testSDF.off",tri::io::Mask::IOM_VERTCOLOR);
  
  cout << "Done SDF" << endl;
  
  adaptor = EmbreeAdaptor<MyMesh>(m5);
  adaptor.computeNormalAnalysis(m5, nOfRays, true);
  tri::io::ExporterOFF<MyMesh>::Save(m5, "testNormal.off", tri::io::Mask::IOM_FACENORMAL);
  //vector<Point3f> BentNormal = adaptor.AOBentNormal(m5,nOfRays);

  cout << "Done NormalAnlysis" << endl;

  adaptor = EmbreeAdaptor<MyMesh>(m6);
  Point3f p(1, 0, 0);
  adaptor.selectVisibleFaces(m6, p, true);
  tri::io::ExporterOFF<MyMesh>::Save(m6, "testSelectS.off", tri::io::Mask::IOM_FACECOLOR);
  
  cout << "done face selection" << endl; 
     

  std::vector<Point3f> origine;
  origine.push_back(Point3f(.0f,10.0f,.0f));
  origine.push_back(Point3f(.0f,10.0f,.0f));
  origine.push_back(Point3f(.0f,10.0f,.0f));
  origine.push_back(Point3f(.0f,10.0f,.0f));
  std::vector<Point3f> direction;
  direction.push_back(Point3f(.0f,1.0f,.0f));
  direction.push_back(Point3f(.0f,1.0f,3.0f));
  direction.push_back(Point3f(.0f,1.0f,6.0f));
  direction.push_back(Point3f(.0f,1.0f,9.0f));
  
  /*
  If you want to test the visualize_ray_shoot add the ply to makefile 

  MyMesh m8;
  
  adaptor.visualize_ray_shoot(m8, origine[0], direction, true);
  int mask = vcg::tri::io::Mask::IOM_VERTCOORD;
  mask |= vcg::tri::io::Mask::IOM_EDGEINDEX;
  tri::io::ExporterPLY<MyMesh>::Save(m8, "EdgeTest.ply", mask);
  */
  cout << "Done All" << endl;

  return 0;
}
