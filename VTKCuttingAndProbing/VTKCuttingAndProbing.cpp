#include "VTKCuttingAndProbing.h"

#include <vtkConeSource.h>
#include <vtkPlaneSource.h>
#include <vtkSTLReader.h>
#include <vtkMultiBlockPLOT3DReader.h>
#include <vtkPlane.h>
#include <vtkCutter.h>
#include <vtkTransform.h>
#include <vtkDataSet.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkStructuredGridGeometryFilter.h>
#include <vtkOutlineFilter.h>
#include <vtkStructuredGridOutlineFilter.h>
#include <vtkProbeFilter.h>
#include <vtkLookupTable.h>
#include <vtkPolyDataMapper.h>
#include <vtkAppendPolyData.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

VTKCuttingAndProbing::VTKCuttingAndProbing(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	_pVTKWidget = new QVTKOpenGLNativeWidget();
	this->setCentralWidget(_pVTKWidget);
	// this->showMaximized();

	// 1. generate data
	// vtkSmartPointer<vtkConeSource> cone = vtkSmartPointer<vtkConeSource>::New();
	// or, read data
	// vtkMultiBlockPLOT3DReader 是一个读取器对象，用于读取 PLOT3D 格式的文件并在输出时生成结构化网格
	vtkSmartPointer<vtkMultiBlockPLOT3DReader> plot3dReader = vtkSmartPointer<vtkMultiBlockPLOT3DReader>::New();
	plot3dReader->SetXYZFileName("combxyz.bin");
	plot3dReader->SetQFileName("combq.bin");
	plot3dReader->SetScalarFunctionNumber(100);
	plot3dReader->SetVectorFunctionNumber(202);
	qDebug() << plot3dReader->GetOutput()->GetNumberOfBlocks(); // 0
	// 反向更新管线
	plot3dReader->Update();
	qDebug() << plot3dReader->GetOutput()->GetNumberOfBlocks(); // 1
	vtkDataSet* plot3dOutput = (vtkDataSet*)(plot3dReader->GetOutput()->GetBlock(0));

	// Cutting
	vtkSmartPointer<vtkPlane> cuttingPlane = vtkSmartPointer<vtkPlane>::New();
	// 设置中心
	cuttingPlane->SetOrigin(plot3dOutput->GetCenter());
	// 设置法向量
	cuttingPlane->SetNormal(1, 0, 0.3);
	vtkSmartPointer<vtkCutter> planeCut = vtkSmartPointer<vtkCutter>::New();
	planeCut->SetInputData(plot3dOutput);
	// 设置截取平面隐函数
	planeCut->SetCutFunction(cuttingPlane);
	double range[2] = { -5, 5 };
	planeCut->GenerateValues(3, range);

	// 截面数据
	vtkSmartPointer<vtkPlaneSource> probePlane = vtkSmartPointer<vtkPlaneSource>::New();
	probePlane->SetResolution(50, 50);

	// 对于一个数据，我们需要坐标变换才能移动它的位置
	vtkSmartPointer<vtkTransform> transform1 = vtkSmartPointer<vtkTransform>::New();
	transform1->Translate(3.7, 0, 28.37);
	transform1->Scale(5, 5, 5);
	transform1->RotateY(90);

	vtkSmartPointer<vtkTransform> transform2 = vtkSmartPointer<vtkTransform>::New();
	transform2->Translate(9.2, 0, 31.2);
	transform2->Scale(5, 5, 5);
	transform2->RotateY(90);

	vtkSmartPointer<vtkTransform> transform3 = vtkSmartPointer<vtkTransform>::New();
	transform3->Translate(13.27, 0, 33.3);
	transform3->Scale(5, 5, 5);
	transform3->RotateY(90);

	// 2. filter
	// 提取作为多边形几何（点，线，表面）的栅格的一部分
	vtkSmartPointer<vtkStructuredGridGeometryFilter> plane = vtkSmartPointer<vtkStructuredGridGeometryFilter>::New();
	plane->SetInputData(plot3dOutput);
	plane->SetExtent(1, 100, 1, 100, 7, 7);
	// 产生结构化栅格边界的一个线轮廓
	vtkSmartPointer<vtkStructuredGridOutlineFilter> outline = vtkSmartPointer<vtkStructuredGridOutlineFilter>::New();
	outline->SetInputData(plot3dOutput);
	// 颜色映射表
	vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
	lut->SetNumberOfColors(256); // 指定颜色映射表中有多少种颜色
	lut->Build();

	// 截面 1
	vtkSmartPointer<vtkTransformPolyDataFilter> tpd1Filter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
	tpd1Filter->SetInputConnection(probePlane->GetOutputPort());
	tpd1Filter->SetTransform(transform1); // 坐标变换
	// 截面 1 的 outline
	vtkSmartPointer<vtkOutlineFilter> outlineTpd1 = vtkSmartPointer<vtkOutlineFilter>::New();
	outlineTpd1->SetInputConnection(tpd1Filter->GetOutputPort());
	// 截面 2
	vtkSmartPointer<vtkTransformPolyDataFilter> tpd2Filter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
	tpd2Filter->SetInputConnection(probePlane->GetOutputPort());
	tpd2Filter->SetTransform(transform2); // 坐标变换
	// 截面 2 的 outline
	vtkSmartPointer<vtkOutlineFilter> outlineTpd2 = vtkSmartPointer<vtkOutlineFilter>::New();
	outlineTpd2->SetInputConnection(tpd2Filter->GetOutputPort());
	// 截面 3
	vtkSmartPointer<vtkTransformPolyDataFilter> tpd3Filter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
	tpd3Filter->SetInputConnection(probePlane->GetOutputPort());
	tpd3Filter->SetTransform(transform3); // 坐标变换
	// 截面 3 的 outline
	vtkSmartPointer<vtkOutlineFilter> outlineTpd3 = vtkSmartPointer<vtkOutlineFilter>::New();
	outlineTpd3->SetInputConnection(tpd3Filter->GetOutputPort());
	// 截面集合
	vtkSmartPointer<vtkAppendPolyData> append = vtkSmartPointer<vtkAppendPolyData>::New();
	append->AddInputConnection(tpd1Filter->GetOutputPort());
	append->AddInputConnection(tpd2Filter->GetOutputPort());
	append->AddInputConnection(tpd3Filter->GetOutputPort());
	// Probe
	vtkSmartPointer<vtkProbeFilter> probeFilter = vtkSmartPointer<vtkProbeFilter>::New();
	probeFilter->SetInputConnection(append->GetOutputPort());
	probeFilter->SetSourceData(plot3dOutput);

	// 3. mapper
	vtkSmartPointer<vtkPolyDataMapper> planeMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	vtkSmartPointer<vtkPolyDataMapper> outlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	vtkSmartPointer<vtkPolyDataMapper> cuttingMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	vtkSmartPointer<vtkPolyDataMapper> outlineTpd1Mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	vtkSmartPointer<vtkPolyDataMapper> outlineTpd2Mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	vtkSmartPointer<vtkPolyDataMapper> outlineTpd3Mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	vtkSmartPointer<vtkPolyDataMapper> probeMapper = vtkSmartPointer<vtkPolyDataMapper>::New();

	// 4. actor
	vtkSmartPointer<vtkActor> planeActor = vtkSmartPointer<vtkActor>::New();
	vtkSmartPointer<vtkActor> outlineActor = vtkSmartPointer<vtkActor>::New();
	vtkSmartPointer<vtkActor> cuttingActor = vtkSmartPointer<vtkActor>::New();
	vtkSmartPointer<vtkActor> outlineTpd1Actor = vtkSmartPointer<vtkActor>::New();
	vtkSmartPointer<vtkActor> outlineTpd2Actor = vtkSmartPointer<vtkActor>::New();
	vtkSmartPointer<vtkActor> outlineTpd3Actor = vtkSmartPointer<vtkActor>::New();
	vtkSmartPointer<vtkActor> probeActor = vtkSmartPointer<vtkActor>::New();

	// 5. renderer
	vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
	renderer->SetBackground(0.3, 0.6, 0.3); // Background Color: Green

	// 6. connect
	planeMapper->SetLookupTable(lut);
	planeMapper->SetInputConnection(plane->GetOutputPort());
	planeMapper->SetScalarRange(plot3dOutput->GetScalarRange()); // 设置标量值的范围
	outlineMapper->SetInputConnection(outline->GetOutputPort());
	cuttingMapper->SetInputConnection(planeCut->GetOutputPort());
	cuttingMapper->SetScalarRange(plot3dOutput->GetScalarRange()); // 设置标量值的范围
	outlineTpd1Mapper->SetInputConnection(outlineTpd1->GetOutputPort());
	outlineTpd2Mapper->SetInputConnection(outlineTpd2->GetOutputPort());
	outlineTpd3Mapper->SetInputConnection(outlineTpd3->GetOutputPort());
	probeMapper->SetInputConnection(probeFilter->GetOutputPort());
	probeMapper->SetScalarRange(plot3dOutput->GetScalarRange()); // 设置标量值的范围
	planeActor->SetMapper(planeMapper);
	outlineActor->SetMapper(outlineMapper);
	cuttingActor->SetMapper(cuttingMapper);
	outlineTpd1Actor->SetMapper(outlineTpd1Mapper);
	outlineTpd2Actor->SetMapper(outlineTpd2Mapper);
	outlineTpd3Actor->SetMapper(outlineTpd3Mapper);
	probeActor->SetMapper(probeMapper);
	// renderer->AddActor(planeActor);
	renderer->AddActor(outlineActor);
	// renderer->AddActor(cuttingActor);
	renderer->AddActor(outlineTpd1Actor);
	renderer->AddActor(outlineTpd2Actor);
	renderer->AddActor(outlineTpd3Actor);
	renderer->AddActor(probeActor);

	this->_pVTKWidget->renderWindow()->AddRenderer(renderer);
	this->_pVTKWidget->renderWindow()->Render();
}

VTKCuttingAndProbing::~VTKCuttingAndProbing()
{}
