// GitPro.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <vtkCommand.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSphereSource.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkClipPolyData.h>
#include <vtkActor.h>
#include <vtkConeSource.h>
#include <vtkPlane.h>
#include <vtkImageActor.h>
#include <vtkImplicitPlaneWidget2.h>
#include <vtkAngleWidget.h>
#include <vtkPlaneWidget.h>
#include <vtkSmartPointer.h>
#include <vtkImplicitPlaneRepresentation.h>
#include <vtkAngleRepresentation3D.h>
#include <vtkPointHandleRepresentation3D.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkJPEGReader.h>
#include <vtkContourFilter.h>
#include <vtkImageFlip.h>
#include <vtkSmoothPolyDataFilter.h>
#include <vtkPolyDataNormals.h>
#include <vtkStripper.h>
#include <vtkProperty.h>
#include <vtkOutlineFilter.h>
#include <vtkCamera.h>
#include <vtkTriangle.h>
#include <vtkImageViewer2.h>
#include <vtkImageData.h>


class VTKImplicitPlaneWidget2Callback : public vtkCommand
{
public:
 static VTKImplicitPlaneWidget2Callback *New()
 {
      return new VTKImplicitPlaneWidget2Callback;
 }
public:
 virtual void Execute(vtkObject *caller, unsigned long eventId, void *callData)
 {
	 vtkImplicitPlaneWidget2 *pWidget = reinterpret_cast<vtkImplicitPlaneWidget2*>(caller);
	 if (pWidget)
	 {
		 vtkImplicitPlaneRepresentation *rep = 
         reinterpret_cast<vtkImplicitPlaneRepresentation*>(pWidget->GetRepresentation());
	     vtkSmartPointer<vtkPlane> planeNew = vtkPlane::New();
         rep->GetPlane(planeNew);
#ifdef GET_VALUE
		 double origin[3];
		 double normal[3];
		 planeNew->GetNormal(normal);
		 planeNew->GetOrigin(origin);
#endif
		 cliper->SetClipFunction(planeNew);
		 cliper->Update();
		 
		 vtkSmartPointer<vtkPolyData> clipedData = vtkPolyData::New();
		 clipedData->DeepCopy(cliper->GetOutput());

		 vtkSmartPointer<vtkPolyDataMapper> coneMapper = vtkPolyDataMapper::New();
		 coneMapper->SetInput(clipedData);
		 coneMapper->ScalarVisibilityOff();	
		 actor->SetMapper(coneMapper);
	 }
 }
 void setCliper(vtkSmartPointer<vtkClipPolyData> other){cliper = other;}
 void setPlane(vtkSmartPointer<vtkPlane> other){pPlane = other;}
 void setActor(vtkSmartPointer<vtkActor> other){actor = other;}
private:
     vtkSmartPointer<vtkPlane> pPlane;
	 vtkSmartPointer<vtkActor> actor;
	 vtkSmartPointer<vtkClipPolyData> cliper;
};

void build3DViewBackup()
{
	vtkSmartPointer<vtkRenderer> aRenderer =
		vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkRenderWindow> renWin =
		vtkSmartPointer<vtkRenderWindow>::New();

	vtkSmartPointer<vtkRenderWindowInteractor> iren =
		vtkSmartPointer<vtkRenderWindowInteractor>::New();
	
	vtkSmartPointer<vtkRenderer> rRenderer = 
	vtkSmartPointer<vtkRenderer>::New();
	
	renWin->AddRenderer(aRenderer);
	renWin->AddRenderer(rRenderer);
	iren->SetRenderWindow(renWin);

	rRenderer->SetBackground( 0.2, 0.3, 0.5 );
	rRenderer->SetViewport(0.5, 0.0, 1.0, 1.0);

	vtkSmartPointer<vtkJPEGReader> dicomReader =
		vtkSmartPointer<vtkJPEGReader>::New();  

	dicomReader->SetFilePrefix("C:/Users/DawnWind/Desktop/004LR/");
	dicomReader->SetFilePattern("%s%d.jpg");
	dicomReader->SetDataByteOrderToLittleEndian();
	dicomReader->SetDataSpacing(2.0 / 3, 2.0 / 3, 1); 
	dicomReader->SetFileNameSliceSpacing(1); 
#ifndef CT_004
	dicomReader->SetDataExtent(0, 511, 0, 209, 0, 37);
#endif
#ifdef CT_000
	dicomReader->SetDataExtent(0, 209, 0, 209, 0, 83);
#endif
	dicomReader->Update();  

	vtkSmartPointer<vtkContourFilter> skinExtractor =
		vtkSmartPointer<vtkContourFilter>::New();

	vtkSmartPointer<vtkImageFlip> flip = vtkSmartPointer<vtkImageFlip>::New();
	// with #include <vtkImageData.h> to translate the type
	flip->SetInput(dicomReader->GetOutput());
	flip->SetFilteredAxes(1);
	skinExtractor->SetInputConnection(flip->GetOutputPort());
	//skinExtractor->SetInputConnection(dicomReader->GetOutputPort());
	skinExtractor->SetValue(0, 100);	


	vtkSmartPointer<vtkSmoothPolyDataFilter> smooth = vtkSmoothPolyDataFilter::New();
	smooth->SetInput( skinExtractor->GetOutput());
	smooth->SetNumberOfIterations( 500 );

	vtkSmartPointer<vtkPolyDataNormals> skinNormals =
		vtkSmartPointer<vtkPolyDataNormals>::New();

	skinNormals->SetInputConnection(smooth->GetOutputPort());
	skinNormals->SetFeatureAngle(60.0);		

	vtkSmartPointer<vtkStripper> skinStripper =		//create triangle strips and/or poly-lines 为了更快的显示速度
		vtkSmartPointer<vtkStripper>::New();
	skinStripper->SetInputConnection(skinNormals->GetOutputPort()); 

	vtkSmartPointer<vtkPolyDataMapper> skinMapper =
		vtkSmartPointer<vtkPolyDataMapper>::New();
	skinMapper->SetInputConnection(skinStripper->GetOutputPort());
	skinMapper->ScalarVisibilityOff();	

	vtkSmartPointer<vtkActor> skin =
		vtkSmartPointer<vtkActor>::New();
	skin->SetMapper(skinMapper); 
	vtkSmartPointer<vtkProperty> backProp = vtkSmartPointer<vtkProperty>::New();
	backProp->SetDiffuseColor(1.0, 0.8, 0.85);
	skin->SetBackfaceProperty(backProp);

	vtkSmartPointer<vtkOutlineFilter> outlineData =
		vtkSmartPointer<vtkOutlineFilter>::New();
	outlineData->SetInputConnection(flip->GetOutputPort());

	vtkSmartPointer<vtkPolyDataMapper> mapOutline =
		vtkSmartPointer<vtkPolyDataMapper>::New();
	mapOutline->SetInputConnection(outlineData->GetOutputPort());

	vtkSmartPointer<vtkActor> outline =
		vtkSmartPointer<vtkActor>::New();
	outline->SetMapper(mapOutline);
	outline->GetProperty()->SetColor(0, 0, 0);
	
	vtkSmartPointer<vtkCamera> aCamera =
		vtkSmartPointer<vtkCamera>::New();
	aCamera->SetViewUp (0, 0, 1);
	aCamera->SetPosition (0, 1, 0);
	aCamera->SetFocalPoint (0, 0, 0);
	aCamera->ComputeViewPlaneNormal();
	aCamera->Azimuth(30.0);
	aCamera->Elevation(30.0);

	aRenderer->AddActor(outline);
	aRenderer->AddActor(skin);
	aRenderer->SetActiveCamera(aCamera);
	aRenderer->ResetCamera();
	aCamera->Dolly(1.5);

	aRenderer->SetBackground(.2, .3, .4);
	renWin->SetSize(640, 480);

	aRenderer->ResetCameraClippingRange ();
	
	vtkSmartPointer<vtkInteractorStyleTrackballCamera> style = 
    vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    iren->SetInteractorStyle( style );
 
	//Use vtkTriangle::ComputeNormal() to compute the plane normal. 
    //Then use the first point as the Origin of the plane.
	double point1[3] = {60, 61, 0};
	double point2[3] = {51, 65, 55};
	double point3[3] = {78, 45, 114};
	double normal[3];
	double verticalPlaneNormal[3] = {0, 0, -1};
	double frontalPlaneNoraml[3]  = {0, 1, 0};  
	// measureNormal = normal
	double measureNormal[3] = {0, 1, 0};

	// calculate the normal with 3 points and set up for a plane
	vtkSmartPointer<vtkPlane> planeNew = vtkPlane::New();
	vtkTriangle::ComputeNormal(point1, point2, point3, normal);
	planeNew->SetOrigin(point1);
	planeNew->SetNormal(normal);

#ifndef Render_2
	vtkSmartPointer<vtkConeSource> cone = vtkConeSource::New();
	cone->SetHeight( 3.0 );
	cone->SetRadius( 1.0 );
	cone->SetResolution( 10 );
	vtkSmartPointer<vtkPolyDataMapper> coneMapper = vtkPolyDataMapper::New();
	coneMapper->SetInputConnection(skinStripper->GetOutputPort());
	coneMapper->ScalarVisibilityOff();	
	vtkSmartPointer<vtkActor> coneSkinActor = vtkActor::New();
	coneSkinActor->SetMapper( coneMapper );
	coneSkinActor->SetBackfaceProperty(backProp);

	vtkSmartPointer<vtkActor> coneOutline =
	vtkSmartPointer<vtkActor>::New();
	coneOutline->SetMapper(mapOutline);
	coneOutline->GetProperty()->SetColor(0,0,0); 

	rRenderer->AddActor(outline);
	rRenderer->AddActor(coneSkinActor);
		
	vtkSmartPointer<vtkCamera> rCamera =
	vtkSmartPointer<vtkCamera>::New();
	rCamera->SetViewUp (0, 0, 1);
	rCamera->SetPosition (0, -1, 0);
	rCamera->SetFocalPoint (0, 0, 0);
	rCamera->ComputeViewPlaneNormal();
	rCamera->Azimuth(30.0);
	rCamera->Elevation(30.0);
	rRenderer->SetActiveCamera(rCamera);
	rRenderer->ResetCamera();

	vtkSmartPointer<vtkClipPolyData> cliper = vtkClipPolyData::New();
	cliper->SetInput(skinStripper->GetOutput());
	cliper->SetClipFunction(planeNew);
	cliper->Update();

	// set up representation for vtkImplicitPlaneWidget2 
	// you could set the normal and origin with the value you want 
	vtkSmartPointer<vtkImplicitPlaneRepresentation> rep = 
    vtkSmartPointer<vtkImplicitPlaneRepresentation>::New();
    rep->SetPlaceFactor(1.25); // This must be set prior to placing the widget
    rep->PlaceWidget(coneSkinActor->GetBounds());
#ifndef SET_VALUE
	rep->SetNormal(measureNormal);
    rep->SetOrigin(point3);
#endif
	vtkSmartPointer<vtkImplicitPlaneWidget2> implicitPlaneWidget = vtkImplicitPlaneWidget2::New();
	implicitPlaneWidget->SetInteractor(iren);
	// this set the representation for the widget thus the normal and origin is transfer to the widget
	implicitPlaneWidget->SetRepresentation(rep);

	vtkSmartPointer<VTKImplicitPlaneWidget2Callback> pCall = VTKImplicitPlaneWidget2Callback::New();
	pCall->setPlane(planeNew);
	pCall->setActor(coneSkinActor);
	pCall->setCliper(cliper);

	implicitPlaneWidget->AddObserver(vtkCommand::EndInteractionEvent, pCall);
    implicitPlaneWidget->On();
#endif
	// Render
    renWin->Render();
	// Initialize the event loop and then start it.
	iren->Initialize();
	
	iren->Start();
}


int _tmain(int argc, _TCHAR* argv[])
{
	build3DViewBackup();
	return 0;
}


