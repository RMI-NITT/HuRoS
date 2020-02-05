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
  // much faster when the image has not chanayPrefiÙIncAMont"~rust1Üt1é:ğæx/&¬çÉÉ_Üs@PÉÉ@óOà@óInitiË±wÿusYIn ``ÿ*Y‰Š‰‚•X!yzjys/Ôbş‘µ @Sysutil s@IsLeap Year$qqrus 	ĞPath Delimite|x17 ätem @AnsiStringi
dVal€idIdent^\Langua(ges+v
st	xt1	1eadByte	ÀList Separator	+oadPac~k€<!R€€t	uL ongDateF`ormatŠ.yNamF€MonDthTim$w@erCase RM@SecsTo#Stamp11„ÕCompPMax CurrencyOK3AIinÑ
iFnÑ
A7Day‰`NŒegÁINewÄTœˆNextChaÀrIndexBÉeNow	I“Null@ˆ5AWideKOutOfMe moryErroB¼ÉQuotedGÀÕ!HRCS!RPRaiseL astOSRRÅ`Win32“À+v˜eDiÖç	%en@¥Fil–”jjRep0laceA=r1ê6JT¥?xOÊâAJÿE
èSafeL oLibrarÖya±EuÉ?S¡lwŠT€LŸ ¥S4etÃat.¡«Shøort!`$Î„âò„"a)q	wExc@eptionáp14%5ObjectpÉX XAlloc!k'trBufˆSizÂCpxci1£`À«Typƒc-dCa"¾pcîÁwLàengthbÍ
¡“ƒ3trCop¢>A
DispoC£pEìEØEnd’FBm¤t1px§'V@arRecx
Lt´rIXLé m$L
O°È®FuiŸtlrL«Î	e“"İLo@g"\Mov”ÿ İà\op&Õ[ÏÀ¦Pwo=ngË'P°ÏÕß`„<PaB…Î(PouÍ&RSca5¼Ÿ²“0ToBxooláO	-³Dœefï”xo;<ToÁFßÓ¿rrãÏfšx154ALÊ9ToqJÏÏtrÿ³ÏÈ®bí
Ñ•a¿o¿õ__@j]FülosJ//Âÿ÷ñ<*ToIo¯e² ü64ïïáÂŸ—Şjï ´Ÿ&?»@ƒ£ÿÏÏÁQ¿·?Ë	øUppÓÄ[ 0$‡¯£t1t149t,â%Ğx$t2Q®³ÌpU9__54$iuc$0R 1%û`ToGUID_qupp |3CÆÏT MetaClasàsrx5_¡)g¸d“x@SÖT™pv?b	x4…="% ÓphiÀ"erRf€›$tIF%÷ïnÀßïDæ	û	ysÂªMessTÖúYysLocaäle)ysP±%ÓE‘ rx11_S YSTEMTIMjETæã@_@å@°$bctN¼XGĞœœou#;Ù+Ex¢Ÿùƒ$ÿ€s@0üLCÅª?5ğFromµïKêKïÂ2ÁÁ´ qrui @Sy sutils@T Language s@IndexOf$qˆLoca lesCallbˆack Tspc	š MultiRea dExclusi veWriteS ynchronizer@-/$bcTtr•v/9d49Bğeginc²;‚|!3lock>erâs2 End¶]Àw.ÈUnbÁ.Onƒ}sm9@WaitForSignal²QD‚C”SimpğleRW¸
¶
S°ÿŒ‘ªŠ×¤	“pÊ	•DcS	Thr |âCğountìzĞ“ ƒQ!Close!;0rp20'š#Infoé…ğDelet¥Hash‡¢¦ÎKOpenŸƒ²Ğ`YttachAßT2cyclâ*©extToFloatbcpvƒ©)Value©) housandS eparatoriimo`AMS tringLPÓ5StampTo DateTrx|19¨? ‚ìM8Sec‚ª¿áTo#€Á"x16`tem@Te	rim…£7¤Ansiï!ÎWideÒLerfÂ1x1>
O¶
RØigho
T€
¿@„rueBool`s*yEn0codeá±usm rOd¶Cu&soê(ryĞTooao `'rÚ@Cur2j_ngr1†5´Qency‹
ÿÅ/#ˆ—$ÿ•,__à-ç;/ &dm@7+rfŸz=´InÏ&ÍÄ&rJ¨÷64zj?	oŸH¯&w oDigitYe arCentur€yWindow8äUnPQdP® ²q	ª³UpperCa#hšN‘3ComĞRgá>+W5t1È(T Z?c>Fmt=ár¶C'w
7pxD14¤.TVaP“cÎxÙÀ”maÌOÑQbCBuâÅpàvuipxB Ÿ,èLow_m¯HÁÌ	Üam/§
îSğÿï‚yaosaa
 n32Build`NumbeirÀ&3 2CSDV@¾ioÂnMChecñÕz2Á2Major_³4in¿sÀ`Pl@atform™rüap¿4Z ¿à·`t1rx2qxÁ% Set$tc$iuc$0R 255%éinitiaàlizatĞÑ*0‰¨TyppèBğ¤d2z2i   @S1:erXPoi3Eà{1ƒTRectUEquÌalò/x19¡y#Fi Á¯!ğ€¬sOğ&Lt2eI‚sEmptyÆ	Offset¯=iÀÃ	&(tI]Àècò7<à 2sTO‚	U)Un v³ Rect$qqr r11Types$@Tˆrx
pt2 @tiniti alizatio
nàv4info @Boolean@Idents,D otSepEP ropertyConv ErroŒr@Finz FreeAndNDil\iesp 14System`@TObj ëfG etEnumNa´meƒ7 q‚cTu:I€hi.’+p1§x17S AnsiStri„ngValuUÁ Float7_›Á^Int64w›aerfacez œ^!Methodÿ· ÜŞÀê ZÅg àaClas{ h|
ÿq
uáâßmÀ‰ÎmŸ—?&1ü|Or_<ı`¥Tá¯˜¾%
&	±46„;%Set$t(˜ Kind@$iuc$0¢ 1Ü7%*’«Å—TÆD?@‹µo@ÿ¦½??$ÑGÁ+a? pa16380$@bÆå*.	Lis"õıNÚrß¡ó uaÇï¹!ypG!@v-şoŸ?€¯4ç˜pì}?ng› .qçO2ÀfoßàFßçĞ¿¾tr¿„‰¿~f¬ÏV¯üƒ'D8ataƒÿÊVağrianÏ‚5”¯
¬
UdcYï
ì
WidŞePÏÏT@P"s¡—o@IsP€ublishe¿\ïE?9rP5OR†@oaStor?çf‰Isïº°/ó)ÁIs#ß.ß."«=dd‡%ı/cÿngÿ/,
4"ßİ§¿ÿßÙ@D±Ã!ÏÀìfomßÂÿdÙZ	/º‰Ï/	 	HÆï¯Uô+·*xoºçódqpxj"ó	ngoºJ/	p45¤%DelxphipcQ‘¤IÿFˆ3/6‚˜/æt//o@ ğCŸÄó7î/foıöãƒ=´ nfo@SetM ethodPro p$qqrp14 System@T Objectx17@AnsiStÀringrx”|À @Typi¢pQ¢p17XT$I1 6t1o)B”t1íArÓi¦>‹AjValue±¨VÀariant
G Ó?)'fQ&(t2ù‹trÿ#‰ŒÀ•à#ÜTo¦H—iK—„X?ª0È¼Qh1œWiüdeÙE~D@ëJnGñg’
ort!)Listâ/a163x80$ÑÉd"‰ 7e£‚q‡TPub lishable“¤/ÀŠe@?Geertyr8TàDatarxF?qät&û	)–in itializationÁv @Àconv@FiÆnU@AsC€`ÿ‚G³@‰ Âà |øxus§ÀÈàCreat¢œó% W%D	(d­IrÂ	•%Ás@Cha@ngeAny RcCÂälearÏD ynArrayF8romàa)pvïsp£ ã	ToÉ#&ã„pxvªEm ptyParamH<Ã`dCust¦¡\!Q.rp2â7s@TÎKo9bus_ÊXNuŒllAyRef_¿€:Ÿ
¢ob<$bc¼tr¡$)Ão@†÷9pdu‚âBÀ6ryOÒ¯APBÎxPLŸ—CaƒPÏ—À¸?
@¡ToQŸÏ‘¿ŠôOl"9/ÿ[%@ComparÓ·Kt1r26 tResuşlŠ·'%ôIËDispInvŒokß[p16Õª CallDescûÊ6/@@Fq@2×o#4"!LefcmoóµY‰bir_‹ğ Hßà¿Quer yInterfacsrx5_GUID[Rai„se%Erro’4_/Š/AtÏDpÜe@°idóqighÁwxƒLO@Simp`wğticC6{/)Ù¨opo%x€¢dã/ Una¿A,›ÿ ±b'&h$ßd?(ïAæõt¯Rntÿo«¿Œ(*-© ¡ªaîCOÏ;_*y ĞÒÁNoInd[/nohQ‚ßtaÇx9²”¯$w	¾¯}nîgÔùI`¤o²iNV´ ype@VarD ataIsArr ay$qqrrx8T¨ xian ts@TCust,om `<T
èByRef3tEmpt@yParam3?Float3:Numeric3Ordinal3Str	1To-_Add	Ísv›å_Rel0easeÀ%‹
In vokeable‡‰~	SDispC‚:†”H—p16System —all`DescpŠ)To FunctionVx17DAnsi@Mingx32Ä%Dynami*cC¼t†&%`5oP€rocedurAJ‹‰Ê¿T“;GetÀğpertCçQR3NSLç~ñ	t1(
U nassigneda0‰6 Dâ*Creat‚$pxixius¨eDimCÈounäw14ä3ÀïaŒ`!b>xmb`HighB@Âr…ÎiPLock“]’wßâ§eO¥påNxqPuc#Í/ ³MUnlrsAÂAsÀ³$-=C0heckÂ¼¿*ar@Compar`lu$lêP *CopàyNoInC<­/%Á”Az«Ens |0Rang–ÍFr„om éeTimbNx‡›ÅkInÿaşraB.
ô­,iùM¾omÈ{oğC bÿ”k¯¬Ò‚ÑÏ!şmu‡¯ë‚okè¯ñ«Nulä|ŸˆƒÿÏËŠ‚ÏËö}‹Cï7pxus½DIÇï:í:Samß7 zôm/SupportsŸ‘rx5_ğGUIDk?B€ç5ª€°Toé5=;ß¸’¿”r5‚Då«½	pngWideÿ4ÏÂôd?‹< ·ß5!"IsàValidy%#ğ!„Element¯Àinitiàzaµ—suti0ls@C qTo±DInf³FóFá²i¿±Ÿ‘clcHom@@V“T	±bonğ
Factpory@p¯£$DbcC·p23pobj@§omSer verObjectpF£TMeta<Cl Å(HaIn stanc°22ÕTh@ÁgMoèdel¯tU
“‘`#c±x45¶©el phiInter@face$tV	IFªƒÓ
‘hs{ÏÏ@È
8ocUpdàRegis<tr‚©ÁÓïv ÿ ÿ±  `     ÀF9O9X9 €ÿÿ€ÿÿÿÿÿÿÿÿÿÿÿÿÿ                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       ¦² MZ    ‚ 0ÿÿ  ¸ 8-@80Ø p º ´	Í!¸ LÍ!This  program  cannot  be run i n DOS mo€de.
$° Ö…Ş’äóA›œa“ ›TœfwgbaRich3N  PE  L â‘Ç[à  !	  P  ´2 0a€	 ĞL  
ó 	   ’ÈyÓ€—@‚˜‚   Œ ¼† ®ƒŠ	  ä   E.teÄxt &LHuŒ?   `.dadta€\ÆÀ	Á>È=ÀÀ À… À€À.prsrc Ã0@Š ŞÈN€À	@.rÈelo 
å À6Ô æÀäÎ	B@¿ ÿ??ÿ????????ÿ????????ÿ????????ÿ???ÿÿÿÿÿÿÿÿÿÿÿÿÿ”±B   â‘Ç[p @# ,  ,   bad a llocatio n  RSDSÒ j?gFHO¹ ÷Tæâ ^msxml6.Àpdb  ¢ÿ€ÿ??ÿ????????ÿ????????ÿ????????ÿ????ÿÿÿÿÿÿÿÿÿÿÿÿÿ°¢ü°¢ü°¢ü°¢ü°¢ü°¢ü°¢ü°¢ü°¢ü°¢ü°¢ü°¢ü°¢ü°¢ü                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            °¢ü°¢ü²¢L 3ÀÃ‹ÿU ‹ì‹E÷Ø À%Á €@ €]Â  8‰>H@W  ( Ì    Ç[ê  U ¸ Ì àU m s p P S *! :U õ     =  M SXML6.DL L DllReg isterServerSetP€ropertyUnrCanU nloadNowGetClas€sObject€3ÿÿ@ ÿ???????ÿ????????ÿ????????ÿ???????ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ° 0ÑLù?° 0ÑLù?° 0ÑLù?° 0ÑLù?° 0ÑLù?° 0ÑLù?° 0ÑLù?° 0ÑLù?° 0ÑLù?° 0ÑLù?° 0ÑLù?° 0ÑLù?° 0ÑLù?                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            