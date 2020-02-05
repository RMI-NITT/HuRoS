/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkRenderer.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkRenderer - abstract specification for renderers
// .SECTION Description
// vtkRenderer provides an abstract specification for renderers. A renderer
// is an object that controls the rendering process for objects. Rendering
// is the process of converting geometry, a specification for lights, and
// a camera view into an image. vtkRenderer also performs coordinate
// transformation between world coordinates, view coordinates (the computer
// graphics rendering coordinate system), and display coordinates (the
// actual screen coordinates on the display device). Certain advanced
// rendering features such as two-sided lighting can also be controlled.

// .SECTION See Also
// vtkRenderWindow vtkActor vtkCamera vtkLight vtkVolume

#ifndef __vtkRenderer_h
#define __vtkRenderer_h

#include "vtkViewport.h"

#include "vtkVolumeCollection.h" // Needed for access in inline members
#include "vtkActorCollection.h" // Needed for access in inline members

class vtkRenderWindow;
class vtkVolume;
class vtkCuller;
class vtkActor;
class vtkActor2D;
class vtkCamera;
class vtkLightCollection;
class vtkCullerCollection;
class vtkLight;
class vtkPainter;
class vtkIdentColoredPainter;
class vtkHardwareSelector;
class vtkRendererDelegate;
class vtkRenderPass;
class vtkTexture;

#if !defined(VTK_LEGACY_REMOVE)
class vtkVisibleCellSelector;
#endif


class VTK_RENDERING_EXPORT vtkRenderer : public vtkViewport
{
public:
  vtkTypeMacro(vtkRenderer,vtkViewport);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create a vtkRenderer with a black background, a white ambient light,
  // two-sided lighting turned on, a viewport of (0,0,1,1), and backface
  // culling turned off.
  static vtkRenderer *New();

  // Description:
  // Add/Remove different types of props to the renderer.
  // These methods are all synonyms to AddViewProp and RemoveViewProp.
  // They are here for convenience and backwards compatibility.
  void AddActor(vtkProp *p);
  void AddVolume(vtkProp *p);
  void RemoveActor(vtkProp *p);
  void RemoveVolume(vtkProp *p);

  // Description:
  // Add a light to the list of lights.
  void AddLight(vtkLight *);

  // Description:
  // Remove a light from the list of lights.
  void RemoveLight(vtkLight *);

  // Description:
  // Remove all lights from the list of lights.
  void RemoveAllLights();

  // Description:
  // Return the collection of lights.
  vtkLightCollection *GetLights();

  // Description:
  // Set the collection of lights.
  // We cannot name it SetLights because of TestSetGet
  // \pre lights_exist: lights!=0
  // \post lights_set: lights==this->GetLights()
  void SetLightCollection(vtkLightCollection *lights);

  // Description:
  // Create and add a light to renderer.
  void CreateLight(void);

  // Description:
  // Create a new Light sutible for use with this type of Renderer.
  // For example, a vtkMesaRenderer should create a vtkMesaLight
  // in this function.   The default is to just call vtkLight::New.
  virtual vtkLight *MakeLight();

  // Description:
  // Turn on/off two-sided lighting of surfaces. If two-sided lighting is
  // off, then only the side of the surface facing the light(s) will be lit,
  // and the other side dark. If two-sided lighting on, both sides of the
  // surface will be lit.
  vtkGetMacro(TwoSidedLighting,int);
  vtkSetMacro(TwoSidedLighting,int);
  vtkBooleanMacro(TwoSidedLighting,int);

  // Description:
  // Turn on/off the automatic repositioning of lights as the camera moves.
  // If LightFollowCamera is on, lights that are designated as Headlights
  // or CameraLights will be adjusted to move with this renderer's camera.
  // If LightFollowCamera is off, the lights will not be adjusted.
  //
  // (Note: In previous versions of vtk, this light-tracking
  // functionality was part of the interactors, not the renderer. For
  // backwards compatibility, the older, more limited interactor
  // behavior is enabled by default. To disable this mode, turn the
  // interactor's LightFollowCamera flag OFF, and leave the renderer's
  // LightFollowCamera flag ON.)
  vtkSetMacro(LightFollowCamera,int);
  vtkGetMacro(LightFollowCamera,int);
  vtkBooleanMacro(LightFollowCamera,int);

  // Description:
  // Turn on/off a flag which disables the automatic light creation capability.
  // Normally in VTK if no lights are associated with the renderer, then a light
  // is automatically created. However, in special circumstances this feature is
  // undesirable, so the following boolean is provided to disable automatic
  // light creation. (Turn AutomaticLightCreation off if you do not want lights
  // to be created.)
  vtkGetMacro(AutomaticLightCreation,int);
  vtkSetMacro(AutomaticLightCreation,int);
  vtkBooleanMacro(AutomaticLightCreation,int);

  // Description:
  // Ask the lights in the scene that are not in world space
  // (for instance, Headlights or CameraLights that are attached to the
  // camera) to update their geometry to match the active camera.
  virtual int UpdateLightsGeometryToFollowCamera(void);

  // Description:
  // Return the collection of volumes.
  vtkVolumeCollection *GetVolumes();

  // Description:
  // Return any actors in this renderer.
  vtkActorCollection *GetActors();

  // Description:
  // Specify the camera to use for this renderer.
  void SetActiveCamera(vtkCamera *);

  // Description:
  // Get the current camera. If there is not camera assigned to the
  // renderer already, a new one is created automatically.
  // This does *not* reset the camera.
  vtkCamera *GetActiveCamera();

  // Description:
  // Create a new Camera sutible for use with this type of Renderer.
  // For example, a vtkMesaRenderer should create a vtkMesaCamera
  // in this function.   The default is to just call vtkCamera::New.
  virtual vtkCamera *MakeCamera();

  // Description:
  // When this flag is off, the renderer will not erase the background
  // or the Zbuffer.  It is used to have overlapping renderers.
  // Both the RenderWindow Erase and Render Erase must be on
  // for the camera to clear the renderer.  By default, Erase is on.
  vtkSetMacro(Erase, int);
  vtkGetMacro(Erase, int);
  vtkBooleanMacro(Erase, int);

  // Description:
  // When this flag is off, render commands are ignored.  It is used to either
  // multiplex a vtkRenderWindow or render only part of a vtkRenderWindow.
  // By default, Draw is on.
  vtkSetMacro(Draw, int);
  vtkGetMacro(Draw, int);
  vtkBooleanMacro(Draw, int);

  // Description:
  // Add an culler to the list of cullers.
  void AddCuller(vtkCuller *);

  // Description:
  // Remove an actor from the list of cullers.
  void RemoveCuller(vtkCuller *);

  // Description:
  // Return the collection of cullers.
  vtkCullerCollection *GetCullers();

  // Description:
  // Set the intensity of ambient lighting.
  vtkSetVector3Macro(Ambient,double);
  vtkGetVectorMacro(Ambient,double,3);

  // Description:
  // Set/Get the amount of time this renderer is allowed to spend
  // rendering its scene. This is used by vtkLODActor's.
  vtkSetMacro(AllocatedRenderTime,double);
  virtual double GetAllocatedRenderTime();

  // Description:
  // Get the ratio between allocated time and actual render time.
  // TimeFactor has been taken out of the render process.
  // It is still computed in case someone finds it useful.
  // It may be taken away in the future.
  virtual double GetTimeFactor();

  // Description:
  // CALLED BY vtkRenderWindow ONLY. End-user pass your way and call
  // vtkRenderWindow::Render().
  // Create an image. This is a superclass method which will in turn
  // call the DeviceRender method of Subclasses of vtkRenderer.
  virtual void Render();

  // Description:
  // Create an image. Subclasses of vtkRenderer must implement this method.
  virtual void DeviceRender() =0;

  // Description:
  // Render translucent polygonal geometry. Default implementation just call
  // UpdateTranslucentPolygonalGeometry().
  // Subclasses of vtkRenderer that can deal with depth peeling must
  // override this method.
  // It updates boolean ivar LastRenderingUsedDepthPeeling.
  virtual void DeviceRenderTranslucentPolygonalGeometry();

  // Description:
  // Clear the image to the background color.
  virtual void Clear() {};

  // Description:
  // Returns the number of visible actors.
  int VisibleActorCount();

  // Description:
  // Returns the number of visible volumes.
  int VisibleVolumeCount();

  // Description:
  // Compute the bounding box of all the visible props
  // Used in ResetCamera() and ResetCameraClippingRange()
  void ComputeVisiblePropBounds( double bounds[6] );

  // Description:
  // Wrapper-friendly version of ComputeVisiblePropBounds
  double *ComputeVisiblePropBounds();

  // Description:
  // Reset the camera clipping range based on the bounds of the
  // visible actors. This ensures that no props are cut off
  void ResetCameraClippingRange();

  // Description:
  // Reset the camera clipping range based on a bounding box.
  // This method is called from ResetCameraClippingRange()
  // If Deering frustrum is used then the bounds get expanded
  // by the camera's modelview matrix.
  void ResetCameraClippingRange( double bounds[6] );
  void ResetCameraClippingRange( double xmin, double xmax,
                                 double ymin, double ymax,
                                 double zmin, double zmax);

  // Description:
  // Specify tolerance for near clipping plane distance to the camera as a
  // percentage of the far clipping plane distance. By default this will be
  // set to 0.01 for 16 bit zbuffers and 0.001 for higher depth z buffers
  vtkSetClampMacro(NearClippingPlaneTolerance,double,0,0.99);
  vtkGetMacro(NearClippingPlaneTolerance,double);

  // Description:
  // Automatically set up the camera based on the visible actors.
  // The camera will reposition itself to view the center point of the actors,
  // and move along its initial view plane normal (i.e., vector defined from
  // camera position to focal point) so that all of the actors can be seen.
  void ResetCamera();

  // Description:
  // Automatically set up the camera based on a specified bounding box
  // (xmin,xmax, ymin,ymax, zmin,zmax). Camera will reposition itself so
  // that its focal point is the center of the bounding box, and adjust its
  // distance and position to preserve its initial view plane normal
  // (i.e., vector defined from camera position to focal point). Note: is
  // the view plane is parallel to the view up axis, the view up axis will
  // be reset to one of the three coordinate axes.
  void ResetCamera(double bounds[6]);

  // Description:
  // Alternative version of ResetCamera(bounds[6]);
  void ResetCamera(double xmin, double xmax, double ymin, double ymax,
                   double zmin, double zmax);

  // Description:
  // Specify the rendering window in which to draw. This is automatically set
  // when the renderer is created by MakeRenderer.  The user probably
  // shouldn't ever need to call this method.
  void SetRenderWindow(vtkRenderWindow *);
  vtkRenderWindow *GetRenderWindow() {return this->RenderWindow;};
  virtual vtkWindow *GetVTKWindow();

  // Description:
  // Turn on/off using backing store. This may cause the re-rendering
  // time to be slightly slower when the view changes. But it is
  // much faster when the image has not chanayPrefi�IncAMont"~rust1�t1�:��x/&����_�s@P��@�O�@�Initi˱w�usYIn ``�*Y�����X!yzjys/�b��� @Sysutil s@IsLeap Year$qqrus 	�Path Delimite|x17 �tem @AnsiStringi
dVal�idIdent�^\Langua(ges+v
st	xt1	1eadByte	�List Separator	+oadPac~k�<!R��t�	uL ongDateF`ormat�.yNamF�MonDthTim$w@erCase RM@SecsTo�#S�tamp11��CompPMax CurrencyOK�3AIin�
iFn�
A7Day�`N�eg�INew�T��NextCha�rIndexB�eNow�	I�Null@�5AWideKOutOfMe moryErroB��QuotedG��!HRCS�!RPR�aiseL�astOSRR�`Win32��+v�eDi��	%en@�Fil��jjRep0laceA=r1�6JT�?xO��AJ�E
�SafeL�oLibrar�ya�Eu�?S�lw�T�L���S4et�at.��Sh�ort!`$΄��"a)q	wExc@eption�p14%5Objectp�X XAlloc!k'trBuf�Siz�Cpxci1�`��Typ�c-dCa"�pc��wL�engthb�
���3trCop�>A�
DispoC�pE�E�End�FBm�t1px�'V@arRecx
Lt�rIXL� m$L
O���Fui�tlrL��	e�"�L�o@g"\Mov�� ��\op&�[���Pwo=ng�'P����`�<PaB��(Pou�&RSca5����0ToBxool�O	-�D�ef����xo;<To�F���rr��f�x154AL�9ToqJ��tr�����b�
��a�o��__@j]F�losJ//����<*ToIo�e����64�������j� ��&?�@������Q��?��	�Upp�č[ 0$����t1t149t,�%�x$t2Q���pU�9__54$i�uc$0R 1%��`ToGUID_qupp |3C��T MetaClas�srx5_�)g�d�x@S�T�pv?b	x4�="% �phi�"erRf��$tIF%��n���D�	�	ysªMessT��YysLoca�le)ysP�%�E� rx11_S YSTEMTIMjET��@_@�@�$bctN�XGМ�ou#;�+Ex�����$��s@0�LCŪ?5�From��K�K��2��� qrui @Sy sutils@T Language s@IndexOf$q�Loca lesCallb�ack Tspc	� MultiRea dExclusi veWriteS ynchronizer@-/$bcTtr�v/9d49B�egin�c�;�|!3lock>er�s2 End�]�w.�Unb�.On�}sm9@WaitFor�Signal�QD�C�Simp�leRW�
�
S������פ�	�p�	�Dc�S	Thr�|�C�ount�z�� �Q!Close!;0rp20'�#Info��Delet�Hash����K�Open����`YttachA�T2cycl�*�extToFloatbcpv��)Value�) housandS eparatoriimo`AMS�tringLP�5StampTo DateTrx|19�?���M8Sec����To#��"x16`tem@Te	rim��7�Ansi�!�Wide�Lerf�1x1>
O�
R�igho�
T�
�@�rueBool`s*yEn0code��usm rOd�Cu&so�(ry�Tooao�`'r�@Cur2j_ngr1�5�Qency�
��/#��$��,__�-�;/ &dm@7+rf�z=�In�&��&rJ��64zj?	o�H�&w oDigitYe arCentur�yWindow8�UnPQdP�� �q	��UpperCa�#h�N�3Com�Rg�>+W5t1�(T� Z?c>Fmt�=�r�C'w
7pxD14�.TVaP�c�x����ma�O�QbCBu��p�vuipxB �,�Low_m�H��	�am/�
�S����yaosaa
 n32Build`Numbeir�&3 2CSDV@�io�nMChec��z2�2Major_�4in�s�`Pl@atform�r�ap�4Z ���`t1rx2qx�% Set$tc$iuc$0R 255%�initia�lizat��*0��Typp�B�d2z2i   @S1:erXPoi3E�{1�TRectUEqu�al�/x19�y#Fi ��!���sO�&Lt2eI�sEmpty���	Offset�=i��	&(tI]��c�7<� 2sTO�	U)Un�v� Rect$qqr r11Types$@T�rx
pt2 @tiniti alizatio
n�v4info @Boolean@Idents,D otSepEP ropertyConv Erro�r@Finz FreeAndNDil\ies�p 14System`@TObj �fG etEnumNa�me�7 q�cT�u:I�hi��.�+p1��x17S AnsiStri�ng�ValuU�� Float7_��^Int64w��aerfacez �^!Method�� �����Z�g��aClas{ h|
�q
u���m���m��?&1�|Or_<�`�T����%
&	�46�;%Set$t(��Kind@$iuc$0� 1�7%*��ŗT�D?@��o@���??$�G�+a? pa16380$�@b��*.	Lis"��N�r���ua���!ypG!�@v-�o�?��4�p�}?ng��.q�O2��fo��F�����tr����~f��V����'D8ata���Va�rianς5��
�
UdcY�
�
Wid�eP����T@P"s��o@IsP�ublishe�\�E?9rP5OR�@oaStor?�f��Is���/�)�Is�#�.�."�=dd�%�/c�ng�/,
4"�������@D��!����fom���d�Z	/���/	 	H���U�+�*xo���dqpxj"�	ngo�J/	p45�%DelxphipcQ��I�F��3/6��/�t//o@ �C���7�/fo����=� nfo@SetM ethodPro p$qqrp14 System@T Objectx17@AnsiSt�ringrx�|� @Typi�pQ�p17XT$I1 6t1o)B�t1�Ar��i�>�A�jValue��V�ariant
G �?�)'fQ&(t2��tr�#�����#�To�H��iK��X?�0ȼQh�1�Wi�de�E~D@�JnG�g�
ort!)List�/a163x80$��d"� 7e���q�TPub lishable��/��e@?Geertyr8T��DatarxF?q�t&�	)�in itialization�v @�conv@Fi�nU@AsC�`��G�@� �� |�xus����Creat���% W%D	(d�Ir�	�%�s@Cha@ngeAny�RcC��lear�D ynArrayF8rom�a)pv��sp� �	To�#&�pxv�Em ptyParamH<�`dCust��\!Q.rp2�7s@T�Ko9bus_�XNu�llAyRef_��:�
�ob<$bc�tr�$)�o@��9pdu��B�6ryOҐ�APB�xPL��Ca�P����?
@�ToQ������Ol"9/�[%@Compar��Kt1r26� tResu�l���'%�I�DispInv�ok�[p16ժ CallDesc��6/@@Fq@2�o#4"!Lefcmo�Y�bir_���H���Quer yInterfac�srx5_GUID�[Rai�se%Erro�4_�/�/At�Dp�e@�id�qigh���wx��LO@Simp`w�ticC6{/)٨opo%x��d�/ Una�A�,�� �b'&h$�d?(�A��t�Rnt�o���(*-�� ��a�CO�;_*y ���NoInd[/�nohQ��ta�x9���$w	��}n�g��I`�o�iNV� ype@VarD ataIsArr ay$qqrrx8T� xian ts@TCust,om `<T
�ByRef3tEmpt@yParam3?Float3:Numeric3Ordinal3Str	1To-_Add	�sv��_Rel0ease�%�
In vokeable��~	SDispC�:��H�p16System �all`Descp�)To FunctionVx17DAnsi@Mingx32�%Dynami*cC�t�&%`5oP�rocedurAJ��ʿT�;Get��pertC�QR3NSL�~�	t1(
U nassigneda0�6�D�*Creat�$pxixius�eDimC�oun�w14�3��a�`!b>xmb`HighB@�r��iPLock�]�w��eO�p�NxqPuc#�/� �M�UnlrsA�As��$-=C0heck¼�*ar@Compar`�lu$l�P�*Cop�yNoInC<�/%���Az�Ens |0Rang��Fr�om �eTimbNx���kIn�a�raB.
��,i�M�om�{o�C b��k��҂��!�mu���ok聯�Nul�|������������}��C�7pxus�DI��:�:Sam�7 z�m/Supports��rx5_�GUIDk?B��5���To�5=;߸���r5�D嫽	�png�Wide��4���d?��<���5!"Is�Validy%#�!�Element��initi�za��suti0ls@C qTo�DInf�F�F�i����clcHom@@V�T	�bon�
Factpory@p��$DbcC�p23pobj@�omSer verObjectpF�TMeta<Cl ��(HaIn stanc�22�Th�@�gMo�del�tU
��`#c�x45��el phiInter@face$tV	IF�����
�hs{��@�
8ocUpd��Regis<tr�����v � ��  `     �F9O9X9 �����������������                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       �� MZ�    � 0��  � 8-@80� p � �	�!� L�!This  program  cannot  be run i n DOS mo�de.
$� օ�ޒ��A��a�� �T�fwgbaRich3N  PE  L ��[�  !	  P  �2 0a�	 �L � 
� 	� � �� ��y���@���� � � � �� ���	  �   E.te�xt &LH�u�?   `.dadta�\��	�>�=�� �� ���.prsrc �0@� ��N��	@.r�elo 
� �6� ����	B@� �??�????????�????????�????????�???���������������B   ��[p @# ,  ,   bad a llocatio n  RSDS� j?gFHO� �T�� ^msxml6.�pdb  ����??�????????�????????�????????�????�������������������������������������������������������                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            ��������L 3�Ë�U ��E�� �%� �@ �]�  8�>H@W  ( �    ��[�  U � � �U m s p P S *! :U �     =  M SXML6.DL L DllReg isterServer�SetP�ropertyUnrCanU nloadNow�GetClas�sObject�3��@ �???????�????????�????????�???????��������������� 0�L�?� 0�L�?� 0�L�?� 0�L�?� 0�L�?� 0�L�?� 0�L�?� 0�L�?� 0�L�?� 0�L�?� 0�L�?� 0�L�?� 0�L�?                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            