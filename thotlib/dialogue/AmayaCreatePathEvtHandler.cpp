#ifdef _WX

#include "wx/wx.h"
#include "wx/event.h"

#include "thot_gui.h"
#include "thot_sys.h"
#include "constmedia.h"
#include "constmedia.h"
#include "typemedia.h"
#include "frame.h"
#include "libmsg.h"
#include "message.h"
#include "typeint.h"
#include "undo.h"

#undef THOT_EXPORT
#define THOT_EXPORT extern
#include "boxes_tv.h"
#include "frame_tv.h"

#include "frame_f.h"
#include "presentationapi_f.h"
#include "structcreation_f.h"
#include "boxlocate_f.h"
#include "geom_f.h"
#include "font_f.h"
#include "content_f.h"

#ifdef _GL
#include "glwindowdisplay.h"
#endif /* _GL */

#include "logdebug.h"


#include "AmayaFrame.h"
#include "AmayaCanvas.h"
#include "AmayaCreatePathEvtHandler.h"

static int N_points, state;
static int currentX,currentY;
static int lastX1, lastY1;
static int symX, symY;
static int lastX2, lastY2;
static int lastX3, lastY3;
static ThotBool clear = FALSE;

/*----------------------------------------------------------------------
  DrawLine
  draw a line from (x2,y2) to (x3,y3)

      (x3,y3)
       .
      .
     .
  (x2,y2)

 *----------------------------------------------------------------------*/
void DrawLine (int x2, int y2, int x3, int y3, ThotBool SpecialColor)
{
  glEnable(GL_COLOR_LOGIC_OP);
  glLogicOp(GL_XOR);

  if(SpecialColor)
    glColor4ub (127, 0, 127, 0);
  else
    glColor4ub (127, 127, 127, 0);

  glBegin(GL_LINE);
  glVertex2i(x2, y2);
  glVertex2i(x3, y3);
  glEnd ();
  glDisable(GL_COLOR_LOGIC_OP);
}

/*----------------------------------------------------------------------
  DrawQuadraticBezier
  draw a quadratic Bezier Curve from (x1,y1) to (x3,y3),
  using (x2,y2) as the control points.

 *----------------------------------------------------------------------*/
void DrawQuadraticBezier (int x1, int y1, int x2, int y2, int x3, int y3,
			  ThotBool SpecialColor)
	      
{
#define N_INTERP 50
  int i;

  GLfloat ctrlpoints[3][3] =
    {{ (float) x1, (float) y1, 0.0}, { (float) x2, (float) y2, 0.0},
     { (float) x3, (float) y3, 0.0}};

  glEnable(GL_COLOR_LOGIC_OP);
  glLogicOp(GL_XOR);

  if(SpecialColor)
    glColor4ub (127, 0, 127, 0);
  else
    glColor4ub (127, 127, 127, 0);

  glEnable(GL_MAP1_VERTEX_3);
  glMap1f(GL_MAP1_VERTEX_3, 0.0, 1.0, 3, 3, ctrlpoints[0]);
  glBegin(GL_LINE_STRIP);

  for (i = 0; i <= N_INTERP; i++)
    glEvalCoord1f((GLfloat) i/((float)N_INTERP));
  glEnd();

  glDisable(GL_MAP1_VERTEX_3);
  glDisable(GL_COLOR_LOGIC_OP);
}

/*----------------------------------------------------------------------
  DrawCubicBezier
  draw a cubic Bezier Curve from (x1,y1) to (x4,y4),
  using (x2,y2) and (x3,y3) as the control points.
 *----------------------------------------------------------------------*/
void DrawCubicBezier (int x1, int y1, int x2, int y2, int x3, int y3,
		      int x4, int y4, ThotBool SpecialColor)
{
  int i;

  GLfloat ctrlpoints[4][3] =
    {{ (float) x1, (float) y1, 0.0}, { (float) x2, (float) y2, 0.0},
     { (float) x3, (float) y3, 0.0}, { (float) x4, (float) y4, 0.0}};

  glEnable(GL_COLOR_LOGIC_OP);
  glLogicOp(GL_XOR);

  if(SpecialColor)
    glColor4ub (127, 0, 127, 0);
  else
    glColor4ub (127, 127, 127, 0);

  glEnable(GL_MAP1_VERTEX_3);
  glMap1f(GL_MAP1_VERTEX_3, 0.0, 1.0, 3, 4, ctrlpoints[0]);
  glBegin(GL_LINE_STRIP);

  for (i = 0; i <= N_INTERP; i++)
    glEvalCoord1f((GLfloat) i/((float)N_INTERP));
  glEnd();

  glDisable(GL_MAP1_VERTEX_3);
  glDisable(GL_COLOR_LOGIC_OP);
}

/*----------------------------------------------------------------------
  UpdateSymetricPoint

  Update the value of the current symetric point.

                               (current)
                           ---- 
           -----(last)-----
       ----
  (sym)

  ----------------------------------------------------------------------*/
void UpdateSymetricPoint()
{
  symX = 2*lastX1 - currentX;
  symY = 2*lastY1 - currentY;
}


/*----------------------------------------------------------------------
  DrawControlPoints

  Draw the control points of the Bezier Curve.
 *----------------------------------------------------------------------*/
void DrawControlPoints ()
{
  glEnable(GL_COLOR_LOGIC_OP);
  glLogicOp(GL_XOR);

  glColor4ub (0, 127, 127, 100);

  glBegin(GL_LINE);
  glVertex2i(symX, symY);
  glVertex2i(currentX, currentY);
  glEnd ();
  glDisable(GL_COLOR_LOGIC_OP);
}


/*----------------------------------------------------------------------
  DrawPathFragment

  Draw the last path fragment. If SpecialColor is TRUE, then this fragment
  is drawn with a special color to show that it is active.
 *----------------------------------------------------------------------*/
void DrawPathFragment(int shape, ThotBool SpecialColor)
{

switch(shape)
  {
  case 5:
  case 6:
    if(state == 1)
      DrawLine (lastX1, lastY1, currentX, currentY, SpecialColor);
    break;

  case 7:
  case 8:
    switch(state)
      {
      case 0:
        /*                                                  */
        /*                                                  */
        /*         (current)+                               */
	break;
      case 1:
        /*                          .....+(current)         */
        /*                    ......                        */
        /*         (last1)O...                              */
	DrawLine (lastX1, lastY1, currentX, currentY, SpecialColor);
	break;
	
      case 2:
        /*                           +(current)             */
        /*  (last2)                 /                       */
        /*        .                /                        */
        /*         ..          .(last1)                     */
        /*           ..     ..  /                           */
        /*             .....   /                            */
        /*                  (sym)                           */
	DrawQuadraticBezier (lastX2, lastY2, symX, symY,
			     lastX1, lastY1, SpecialColor);
	if(SpecialColor)DrawControlPoints();
	break;

      case 3:
        /*                    +(current)                    */
        /*                    ...                           */
        /*                       .                          */
        /*                      .                           */
        /*                  ...   (last1)                   */
        /*             .....                                */
        /*   (last2)...                                     */
        /*                                                  */
	DrawQuadraticBezier (lastX2, lastY2, lastX1, lastY1,
			     currentX, currentY, SpecialColor);
	break;

      case 4:
        /*      (current)+---(last1)---(sym)                */
        /*                      ...                         */
        /*                         ....                     */
        /*                            .                     */
        /*                            .                     */
        /*                          ...                     */
        /*               ...........     (last2)            */
        /*    (last3)....                                   */
	DrawCubicBezier(lastX3, lastY3, lastX2, lastY2, symX, symY,
			lastX1, lastY1, SpecialColor);
	if(SpecialColor)DrawControlPoints();
	break;
      }
    break;

  default:
    break;
  }
}

IMPLEMENT_DYNAMIC_CLASS(AmayaCreatePathEvtHandler, wxEvtHandler)

/*----------------------------------------------------------------------
 *  this is where the event table is declared
 *  the callbacks are assigned to an event type
 *----------------------------------------------------------------------*/
BEGIN_EVENT_TABLE(AmayaCreatePathEvtHandler, wxEvtHandler)
EVT_CHAR( AmayaCreatePathEvtHandler::OnChar )

EVT_LEFT_DOWN(	AmayaCreatePathEvtHandler::OnMouseDown) // Process a wxEVT_LEFT_DOWN event.
EVT_LEFT_UP(		AmayaCreatePathEvtHandler::OnMouseUp) // Process a wxEVT_LEFT_UP event.
EVT_LEFT_DCLICK(	AmayaCreatePathEvtHandler::OnMouseDbClick) // Process a wxEVT_LEFT_DCLICK event.
EVT_MIDDLE_DOWN(	AmayaCreatePathEvtHandler::OnMouseDown) // Process a wxEVT_MIDDLE_DOWN event.
EVT_MIDDLE_UP(	AmayaCreatePathEvtHandler::OnMouseUp) // Process a wxEVT_MIDDLE_UP event.
EVT_MIDDLE_DCLICK(	AmayaCreatePathEvtHandler::OnMouseDbClick) // Process a wxEVT_MIDDLE_DCLICK event.
EVT_RIGHT_DOWN(	AmayaCreatePathEvtHandler::OnMouseDown) // Process a wxEVT_RIGHT_DOWN event.
EVT_RIGHT_UP(		AmayaCreatePathEvtHandler::OnMouseUp) // Process a wxEVT_RIGHT_UP event.
EVT_RIGHT_DCLICK(	AmayaCreatePathEvtHandler::OnMouseDbClick) // Process a wxEVT_RIGHT_DCLICK event.
EVT_MOTION(		AmayaCreatePathEvtHandler::OnMouseMove) // Process a wxEVT_MOTION event.
EVT_MOUSEWHEEL(	AmayaCreatePathEvtHandler::OnMouseWheel) // Process a wxEVT_MOUSEWHEEL event.
END_EVENT_TABLE()

/*----------------------------------------------------------------------
 *----------------------------------------------------------------------*/
AmayaCreatePathEvtHandler::AmayaCreatePathEvtHandler() : wxEvtHandler()
{
}

/*----------------------------------------------------------------------
 *----------------------------------------------------------------------*/
AmayaCreatePathEvtHandler::AmayaCreatePathEvtHandler(AmayaFrame * p_frame, int x_min, int y_min, int x_max, int y_max,  PtrTextBuffer Pbuffer, Document doc, int shape_number)
  : wxEvtHandler()
  ,m_IsFinish(false)
  ,m_pFrame(p_frame)
  ,m_FrameId(p_frame->GetFrameId())
  ,m_xmin(x_min)
  ,m_ymin(y_min)
  ,m_xmax(x_max)
  ,m_ymax(y_max)
  ,m_ShapeNumber(shape_number)
  ,m_Pbuffer(Pbuffer)
  ,m_document(doc)

{
  if (m_pFrame)
    {
      /* attach this handler to the canvas */
      AmayaCanvas * p_canvas = m_pFrame->GetCanvas();
      p_canvas->PushEventHandler(this);

      /* assign a cross mouse cursor */
      m_pFrame->GetCanvas()->SetCursor( wxCursor(wxCURSOR_CROSS) );
      m_pFrame->GetCanvas()->CaptureMouse();
    }

  glDisable(GL_LINE_STIPPLE);
  N_points = 1;
  state = 0;
}

/*----------------------------------------------------------------------
 *----------------------------------------------------------------------*/
AmayaCreatePathEvtHandler::~AmayaCreatePathEvtHandler()
{
  PtrTextBuffer       pB;
  int i, step;
  ThotBool first_point = TRUE;
  
  i = 1;
  step = 0;
  for(pB = m_Pbuffer; pB; pB = pB -> BuNext)
    {
      for(; i < pB->BuLength; i++)
	{

	  /* Convert the points of the polyline/curve */
	MouseCoordinatesToSVG(m_document, m_pFrame,
			      m_xmin, m_xmax, m_ymin, m_ymax, TRUE,
			 &(pB->BuPoints[i].XCoord), &(pB->BuPoints[i].YCoord));

	/* Clear the preview of the polyline/curve */
	if(!first_point)
	  {
	    if(m_ShapeNumber == 5 || m_ShapeNumber == 6)
	      {
		DrawLine (pB->BuPoints[i-1].XCoord, pB->BuPoints[i-1].YCoord,
			  pB->BuPoints[i].XCoord, pB->BuPoints[i].YCoord,
			  FALSE);
	      }
	    else
	      {
		if(step % 3 == 0)
	  DrawCubicBezier (pB->BuPoints[i-3].XCoord,
			   pB->BuPoints[i-3].YCoord,
			   pB->BuPoints[i-2].XCoord,
			   pB->BuPoints[i-2].YCoord,
			   pB->BuPoints[i-1].XCoord,
			   pB->BuPoints[i-1].YCoord,   
			   pB->BuPoints[i].XCoord,
			   pB->BuPoints[i].YCoord, FALSE);
	      }
	  }
	else
	  first_point = FALSE;

	step=(step+1)%3;
	}
      i = 0;
    }

  if (m_pFrame)
    {
      /* detach this handler from the canvas (restore default behaviour) */
      AmayaCanvas * p_canvas = m_pFrame->GetCanvas();
      p_canvas->PopEventHandler(false /* do not delete myself */);

      /* restore the default cursor */
      m_pFrame->GetCanvas()->SetCursor( wxNullCursor );

      m_pFrame->GetCanvas()->ReleaseMouse();
    }
}

/*----------------------------------------------------------------------
 *----------------------------------------------------------------------*/
bool AmayaCreatePathEvtHandler::IsFinish()
{
  return m_IsFinish;
}

/*----------------------------------------------------------------------
 *----------------------------------------------------------------------*/
void AmayaCreatePathEvtHandler::OnChar( wxKeyEvent& event )
{
  m_IsFinish = true;
}

/*----------------------------------------------------------------------
 *       Class:  AmayaCreatePathEvtHandler
 *      Method:  OnMouseDown
 * Description:  handle mouse button down events
 -----------------------------------------------------------------------*/
void AmayaCreatePathEvtHandler::OnMouseDown( wxMouseEvent& event )
{
}

/*----------------------------------------------------------------------
 *       Class:  AmayaCreatePathEvtHandler
 *      Method:  OnMouseUp
 * Description:  handle mouse button up events
 -----------------------------------------------------------------------*/
void AmayaCreatePathEvtHandler::OnMouseUp( wxMouseEvent& event )
{
  if (IsFinish())
    return;

  /* Are we in the SVG ? */
  if(!MouseCoordinatesToSVG(m_document, m_pFrame, m_xmin, m_xmax, m_ymin, m_ymax,
			    FALSE, &currentX, &currentY))return;

  if(m_ShapeNumber == 7 || m_ShapeNumber == 8)
    {
      clear = FALSE;

      if(state == 0 || state == 1 || state == 3)
	{
	  /* Clear the active fragment */
	  DrawPathFragment(m_ShapeNumber, TRUE);
	}
      else
	{
	  /* Clear the active fragment */
	  DrawPathFragment(m_ShapeNumber, TRUE);

	  /* Draw the new curve fragment */
	  DrawPathFragment(m_ShapeNumber, FALSE);
	}
    }
  else
    {
      /* Clear the active fragment */
      DrawPathFragment(m_ShapeNumber, TRUE);
      
      /* Draw the new curve fragment */
      DrawPathFragment(m_ShapeNumber, FALSE);
    }

  if(m_ShapeNumber == 5 || m_ShapeNumber == 6)
    {
      /* Add a new point in the polyline/polygon */
      state = 1;
      AddPointInPolyline (m_Pbuffer, N_points, currentX, currentY);N_points++;
    }
  else if(m_ShapeNumber == 7 || m_ShapeNumber == 8)
    {
      /* Bezier Curve */
      
      if(state == 2)
	{
	  /* Add a cubic Bezier Curve (the control points are the same, so
	     actually it is a quadratic Bezier curve) */
	  AddPointInPolyline (m_Pbuffer, N_points, lastX2, lastY2);N_points++;
	  AddPointInPolyline (m_Pbuffer, N_points, symX, symY);N_points++;
	  AddPointInPolyline (m_Pbuffer, N_points, symX, symY);N_points++;
	  AddPointInPolyline (m_Pbuffer, N_points, lastX1, lastY1);N_points++;
	}
      else if(state == 4)
	{
	  /* Add a cubic Bezier Curve */
	  AddPointInPolyline (m_Pbuffer, N_points, lastX2, lastY2);N_points++;
	  AddPointInPolyline (m_Pbuffer, N_points, symX, symY);N_points++;
	  AddPointInPolyline (m_Pbuffer, N_points, lastX1, lastY1);N_points++;
	}

      if(state < 4)state++;
      else state=3;
    }

  lastX3 = lastX2;
  lastY3 = lastY2;
  lastX2 = lastX1;
  lastY2 = lastY1;
  lastX1 = currentX;
  lastY1 = currentY;
  UpdateSymetricPoint();

}


/*----------------------------------------------------------------------
 *       Class:  AmayaCreatePathEvtHandler
 *      Method:  OnMouseDbClick
 * Description:  handle mouse dbclick events
 -----------------------------------------------------------------------*/
void AmayaCreatePathEvtHandler::OnMouseDbClick( wxMouseEvent& event )
{
  m_IsFinish = true;
}

/*----------------------------------------------------------------------
 *       Class:  AmayaCreatePathEvtHandler
 *      Method:  OnMouseMove
 * Description:  handle mouse move events
 -----------------------------------------------------------------------*/
void AmayaCreatePathEvtHandler::OnMouseMove( wxMouseEvent& event )
{
  if(clear)
    DrawPathFragment(m_ShapeNumber, TRUE);

  currentX = event.GetX();
  currentY = event.GetY();
  UpdateSymetricPoint();

  DrawPathFragment(m_ShapeNumber, TRUE);
  clear = TRUE;

  MouseCoordinatesToSVG(m_document, m_pFrame, m_xmin, m_xmax, m_ymin, m_ymax,
			FALSE, &currentX, &currentY);

  m_pFrame->GetCanvas()->Refresh();

}

/*----------------------------------------------------------------------
 *       Class:  AmayaCreatePathEvtHandler
 *      Method:  OnMouseWheel
 * Description:  handle mouse wheel events
 -----------------------------------------------------------------------*/
void AmayaCreatePathEvtHandler::OnMouseWheel( wxMouseEvent& event )
{
}

#endif /* _WX */