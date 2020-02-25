/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkRenderWindowInteractor.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkRenderWindowInteractor - platform-independent render window
// interaction including picking and frame rate control.

// .SECTION Description
// vtkRenderWindowInteractor provides a platform-independent interaction
// mechanism for mouse/key/time events. It serves as a base class for
// platform-dependent implementations that handle routing of mouse/key/timer
// messages to vtkInteractorObserver and its subclasses. vtkRenderWindowInteractor 
// also provides controls for picking, rendering frame rate, and headlights.
//
// vtkRenderWindowInteractor has changed from previous implementations and
// now serves only as a shell to hold user preferences and route messages to
// vtkInteractorStyle. Callbacks are available for many events.  Platform
// specific subclasses should provide methods for manipulating timers,
// TerminateApp, and an event loop if required via
// Initialize/Start/Enable/Disable.

// .SECTION Caveats
// vtkRenderWindowInteractor routes events through VTK's command/observer
// design pattern. That is, when vtkRenderWindowInteractor (actually, one of
// its subclasses) sees a platform-dependent event, it translates this into
// a VTK event using the InvokeEvent() method. Then any vtkInteractorObservers
// registered for that event are expected to respond as appropriate.

// .SECTION See Also
// vtkInteractorObserver

#ifndef __vtkRenderWindowInteractor_h
#define __vtkRenderWindowInteractor_h

#include "vtkObject.h"

class vtkTimerIdMap;

// Timer flags for win32/X compatibility
#define VTKI_TIMER_FIRST  0
#define VTKI_TIMER_UPDATE 1

class vtkAbstractPicker;
class vtkAbstractPropPicker;
class vtkInteractorObserver;
class vtkRenderWindow;
class vtkRenderer;
class vtkObserverMediator;
class vtkInteractorEventRecorder;

class VTK_RENDERING_EXPORT vtkRenderWindowInteractor : public vtkObject
{
//BTX
  friend class vtkInteractorEventRecorder;
//ETX
public:
  static vtkRenderWindowInteractor *New();
  vtkTypeMacro(vtkRenderWindowInteractor,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Prepare for handling events. This must be called before the
  // interactor will work.
  virtual void Initialize();
  void ReInitialize() {  this->Initialized = 0; this->Enabled = 0;
                        this->Initialize(); } 

  // Description:
  // This Method detects loops of RenderWindow-Interactor,
  // so objects are freed properly.
  virtual void UnRegister(vtkObjectBase *o);

  // Description:
  // Start the event loop. This is provided so that you do not have to
  // implement your own event loop. You still can use your own
  // event loop if you want. Initialize should be called before Start.
  virtual void Start() {}

  // Description:
  // Enable/Disable interactions.  By default interactors are enabled when
  // initialized.  Initialize() must be called prior to enabling/disabling
  // interaction. These methods are used when a window/widget is being
  // shared by multiple renderers and interactors.  This allows a "modal"
  // display where one interactor is active when its data is to be displayed
  // and all other interactors associated with the widget are disabled
  // when their data is not displayed.
  virtual void Enable() { this->Enabled = 1; this->Modified();}
  virtual void Disable() { this->Enabled = 0; this->Modified();}
  vtkGetMacro(Enabled, int);

  // Description:
  // Enable/Disable whether vtkRenderWindowInteractor::Render() calls
  // this->RenderWindow->Render().
  vtkBooleanMacro(EnableRender, bool);
  vtkSetMacro(EnableRender, bool);
  vtkGetMacro(EnableRender, bool);

  // Description:
  // Set/Get the rendering window being controlled by this object.
  void SetRenderWindow(vtkRenderWindow *aren);
  vtkGetObjectMacro(RenderWindow,vtkRenderWindow);

  // Description:
  // Event loop notification member for window size change.
  // Window size is measured in pixels.
  virtual void UpdateSize(int x,int y);

  // Description:
  // This class provides two groups of methods for manipulating timers.  The
  // first group (CreateTimer(timerType) and DestroyTimer()) implicitly use
  // an internal timer id (and are present for backward compatibility). The
  // second group (CreateRepeatingTimer(long),CreateOneShotTimer(long),
  // ResetTimer(int),DestroyTimer(int)) use timer ids so multiple timers can
  // be independently managed. In the first group, the CreateTimer() method
  // takes an argument indicating whether the timer is created the first time
  // (timerType==VTKI_TIMER_FIRST) or whether it is being reset
  // (timerType==VTKI_TIMER_UPDATE). (In initial implementations of VTK this
  // was how one shot and repeating timers were managed.) In the second
  // group, the create methods take a timer duration argument (in
  // milliseconds) and return a timer id. Thus the ResetTimer(timerId) and
  // DestroyTimer(timerId) methods take this timer id and operate on the
  // timer as appropriate. Methods are also available for determining
  virtual int CreateTimer(int timerType); //first group, for backward compatibility
  virtual int DestroyTimer(); //first group, for backward compatibility

  // Description:
  // Create a repeating timer, with the specified duration (in milliseconds).
  // \return the timer id.
  int CreateRepeatingTimer(unsigned long duration);

  // Description:
  // Create a one shot timer, with the specified duretion (in milliseconds).
  // \return the timer id.
  int CreateOneShotTimer(unsigned long duration);

  // Description:
  // Query whether the specified timerId is a one shot timer.
  // \return 1 if the timer is a one shot timer.
  int IsOneShotTimer(int timerId);

  // Description:
  // Get the duration (in milliseconds) for the specified timerId.
  unsigned long GetTimerDuration(int timerId);

  // Description:
  // Reset the specified timer.
  int ResetTimer(int timerId);

  // Description:
  // Destroy the timer specified by timerId.
  // \return 1 if the timer was destroyed.
  int DestroyTimer(int timerId);

  // Description:
  // Get the VTK timer ID that corresponds to the supplied platform ID.
  virtual int GetVTKTimerId(int platformTimerId);

  //BTX
  // Moved into the public section of the class so that classless timer procs
  // can access these enum members without being "friends"...
  enum {OneShotTimer=1,RepeatingTimer};
  //ETX

  // Description:
  // Specify the default timer interval (in milliseconds). (This is used in
  // conjunction with the timer methods described previously, e.g.,
  // CreateTimer() uses this value; and CreateRepeatingTimer(duration) and
  // CreateOneShotTimer(duration) use the default value if the parameter
  // "duration" is less than or equal to zero.) Care must be taken when
  // adjusting the timer interval from the default value of 10
  // milliseconds--it may adversely affect the interactors.
  vtkSetClampMacro(TimerDuration,unsigned long,1,100000);
  vtkGetMacro(TimerDuration,unsigned long);

  // Description:
  // These methods are used to communicate information about the currently
  // firing CreateTimerEvent or DestroyTimerEvent. The caller of
  // CreateTimerEvent sets up TimerEventId, TimerEventType and
  // TimerEventDuration. The observer of CreateTimerEvent should set up an
  // appropriate platform specific timer based on those values and set the
  // TimerEventPlatformId before returning. The caller of Des��������������������������������                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                ��������������������������������                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                ��������������������������������                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                ��������������������
%�d��'����C
@B������ʚ;��% ���~��o�#��]xEc  d����
�Ua>� a� a]��Ed� !R� �]� F d�a��� a �� ��i �