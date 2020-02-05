/* ATK -  Accessibility Toolkit
 * Copyright 2001 Sun Microsystems Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#if defined(ATK_DISABLE_SINGLE_INCLUDES) && !defined (__ATK_H_INSIDE__) && !defined (ATK_COMPILATION)
#error "Only <atk/atk.h> can be included directly."
#endif

#ifndef __ATK_STATE_H__
#define __ATK_STATE_H__

#include <glib-object.h>

G_BEGIN_DECLS

/**
 *AtkStateType:
 *@ATK_STATE_INVALID: Indicates an invalid state - probably an error condition.
 *@ATK_STATE_ACTIVE: Indicates a window is currently the active window, or is an active subelement within a container or table
 *@ATK_STATE_ARMED: Indicates that the object is 'armed', i.e. will be activated by if a pointer button-release event occurs within its bounds.  Buttons often enter this state when a pointer click occurs within their bounds, as a precursor to activation.
 *@ATK_STATE_BUSY:  Indicates the current object is busy, i.e. onscreen representation is in the process of changing, or the object is temporarily unavailable for interaction due to activity already in progress.  This state may be used by implementors of Document to indicate that content loading is underway.  It also may indicate other 'pending' conditions; clients may wish to interrogate this object when the ATK_STATE_BUSY flag is removed.
 *@ATK_STATE_CHECKED: Indicates this object is currently checked, for instance a checkbox is 'non-empty'.
 *@ATK_STATE_DEFUNCT: Indicates that this object no longer has a valid backing widget (for instance, if its peer object has been destroyed)
 *@ATK_STATE_EDITABLE: Indicates the user can change the contents of this object
 *@ATK_STATE_ENABLED: 	Indicates that this object is enabled, i.e. that it currently reflects some application state. Objects that are "greyed out" may lack this state, and may lack the STATE_SENSITIVE if direct user interaction cannot cause them to acquire STATE_ENABLED. See also: ATK_STATE_SENSITIVE
 *@ATK_STATE_EXPANDABLE: Indicates this object allows progressive disclosure of its children
 *@ATK_STATE_EXPANDED: Indicates this object its expanded - see ATK_STATE_EXPANDABLE above
 *@ATK_STATE_FOCUSABLE: Indicates this object can accept keyboard focus, which means all events resulting from typing on the keyboard will normally be passed to it when it has focus
 *@ATK_STATE_FOCUSED: Indicates this object currently has the keyboard focus
 *@ATK_STATE_HORIZONTAL: Indicates the orientation of this object is horizontal; used, for instance, by objects of ATK_ROLE_SCROLL_BAR.  For objects where vertical/horizontal orientation is especially meaningful.
 *@ATK_STATE_ICONIFIED: Indicates this object is minimized and is represented only by an icon
 *@ATK_STATE_MODAL: Indicates something must be done with this object before the user can interact with an object in a different window
 *@ATK_STATE_MULTI_LINE: Indicates this (text) object can contain multiple lines of text
 *@ATK_STATE_MULTISELECTABLE: Indicates this object allows more than one of its children to be selected at the same time, or in the case of text objects, that the object supports non-contiguous text selections.
 *@ATK_STATE_OPAQUE: Indicates this object paints every pixel within its rectangular region.
 *@ATK_STATE_PRESSED: Indicates this object is currently pressed; c.f. ATK_STATE_ARMED
 *@ATK_STATE_RESIZABLE: Indicates the size of this object is not fixed
 *@ATK_STATE_SELECTABLE: Indicates this object is the child of an object that allows its children to be selected and that this child is one of those children that can be selected
 *@ATK_STATE_SELECTED: Indicates this object is the child of an object that allows its children to be selected and that this child is one of those children that has been selected
 *@ATK_STATE_SENSITIVE: Indicates this object is sensitive, e.g. to user interaction. 
 * STATE_SENSITIVE usually accompanies STATE_ENABLED for user-actionable controls,
 * but may be found in the absence of STATE_ENABLED if the current visible state of the 
 * control is "disconnected" from the application state.  In such cases, direct user interaction
 * can often result in the object gaining STATE_SENSITIVE, for instance if a user makes 
 * an explicit selection using an object whose current state is ambiguous or undefined.
 * @see STATE_ENABLED, STATE_INDETERMINATE.
 *@ATK_STATE_SHOWING: Indicates this object, the object's parent, the object's parent's parent, and so on, 
 * are all 'shown' to the end-user, i.e. subject to "exposure" if blocking or obscuring objects do not interpose
 * between this object and the top of the window stack.
 *@ATK_STATE_SINGLE_LINE: Indicates this (text) object can contain only a single line of text
 *@ATK_STATE_STALE: Indicates that the information returned for this object may no longer be
 * synchronized with the application state.  This is implied if the object has STATE_TRANSIENT,
 * and can also occur towards the end of the object peer's lifecycle. It can also be used to indicate that 
 * the index associated with this object has changed since the user accessed the object (in lieu of
 * "index-in-parent-changed" events).
 *@ATK_STATE_TRANSIENT: Indicates this object is transient, i.e. a snapshot which may not emit events when its
 * state changes.  Data from objects with ATK_STATE_TRANSIENT should not be cached, since there may be no
 * notification given when the cached data becomes obsolete.
 *@ATK_STATE_VERTICAL: Indicates the orientation of this object is vertical
 *@ATK_STATE_VISIBLE: Indicates this object is visible, e.g. has been explicitly marked for exposure to the user.
 * @note: STATE_VISIBLE is no guarantee that the object is actually unobscured on the screen, only
 * that it is 'potentially' visible, barring obstruction, being scrolled or clipped out of the 
 * field of view, or having an ancestor container that has not yet made visible.
 * A widget is potentially onscreen if it has both STATE_VISIBLE and STATE_SHOWING.
 * The absence of STATE_VISIBLE and STATE_SHOWING is semantically equivalent to saying 
 * that an object is 'hidden'.  See also STATE_TRUNCATED, which applies if a VISIBLE and SHOWING object
 * lies within a viewport which means that its contents are clipped, e.g. a truncated spreadsheet cell or
 * an image within a scrolling viewport.  Mostly useful for screen-review and magnification algorithms.
 *@ATK_STATE_MANAGES_DESCENDANTS: Indicates that "active-descendant-changed" event
 * is sent when children become 'active' (i.e. are selected or navigated to onscreen).
 * Used to prevent need to enumerate all children in very large containers, like tables.
 * The presence of STATE_MANAGES_DESCENDANTS is an indication to the client.
 * that the children should not, and need not, be enumerated by the client.
 * Objects implementing this state are expected to provide relevant state
 * notifications to listening clients, for instance notifications of visibility
 * changes and activation of their contained child objects, without the client 
 * having previously requested references to those children.
 *@ATK_STATE_INDETERMINATE: Indicates that a check box is in a state other than checked or not checked.
 * This usually means that the boolean value reflected or 
 * controlled by the object does not apply consistently to the entire current context.
 * For example, a checkbox for the "Bold" attribute of text may have STATE_INDETERMINATE
 * if the currently selected text contains a mixture of weight attributes.  
 * In many cases interacting wi                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     