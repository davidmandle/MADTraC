/* Include necessary MADTraC headers */
#include "MT_Core.h"
#include "MT_GUI.h"
#include "MT_Tracking.h"

class SimpleBWTracker : public MT_TrackerBase
{
private:
    /* frames */
    IplImage* m_pGSFrame;      /* Grayscale version of current frame */
    IplImage* m_pDiffFrame;    /* Background subtracted frame */
    IplImage* m_pThreshFrame;  /* Thresholded frame */
    IplImage* m_pOrgFrame;     /* Copy of the original frame */

    /* blobber parameters */
    unsigned int m_iBlobValThresh;
    double m_dMinBlobPerimeter;
    double m_dMinBlobArea;
    double m_dMaxBlobPerimeter;
    double m_dMaxBlobArea;

    /* drawing parameters */
    bool m_bDrawBlobs;
    MT_Color m_BlobColor;

    /* output variables */
    CBlobResult m_Blobs;
    std::vector<double> m_vdBlobXs;
    std::vector<double> m_vdBlobYs;
    std::vector<double> m_vdBlobPs;
    std::vector<double> m_vdBlobAs;
    std::vector<double> m_vdBlobOs;

    /* operational variables */
    double m_dDt;
    int m_iNBlobsFound;
    int m_iFrameCounter;
    int m_iNChannelsPerFrame;
    
public:
    /* constructor */
    SimpleBWTracker(IplImage* ProtoFrame);
    /* destructor - note we need the virtual destructor */
    virtual ~SimpleBWTracker(){};
    
    /* Initialization */
    void doInit(IplImage* ProtoFrame);

    /* Memory allocation / deallocation */
    void createFrames();
    void releaseFrames();

    /* Main tracking functions */
    virtual void doTracking(IplImage* frame);
    virtual void doImageProcessing();
    virtual void doSegmentation();

    /* Drawing function */
    virtual void doGLDrawing(int flags = MT_TB_NO_FLAGS);

};

/**********************************************************************
 * GUI Frame Class
 *********************************************************************/

class SimpleBWTrackerFrame : public MT_TrackerFrameBase
{
protected:
    SimpleBWTracker* m_pSimpleBWTracker;

public:
    SimpleBWTrackerFrame(wxFrame* parent,
                         wxWindowID id = wxID_ANY,
                         const wxString& title = wxT("Tracker View"), 
                         const wxPoint& pos = wxDefaultPosition, 
                         const wxSize& size = wxSize(640,480),     
                         long style = MT_FIXED_SIZE_FRAME);

    virtual ~SimpleBWTrackerFrame(){};

    void initTracker();
    void initUserData();
};

/**********************************************************************
 * GUI App Class
 *********************************************************************/

class SimpleBWTrackerApp
: public MT_AppBase
{
public:
    MT_FrameWithInit* createMainFrame()
    {
        return new SimpleBWTrackerFrame(NULL);
    };
};


