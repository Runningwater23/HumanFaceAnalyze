
// ClientDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Client.h"
#include "ClientDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CClientDlg 对话框



CClientDlg::CClientDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CClientDlg::IDD, pParent)
	, m_edtInfoShow(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EdtInfo, m_edtInfoShow);
}

BEGIN_MESSAGE_MAP(CClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BtnOpen, &CClientDlg::OnBnClickedBtnopen)
	ON_BN_CLICKED(IDC_BtnPause, &CClientDlg::OnBnClickedBtnpause)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CClientDlg 消息处理程序

BOOL CClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}
//////////////////////////////
/*
输入：IplImage *img //要显示的图像
输入：int id //显示图像的控件的id号
*/
//////////////////////////////
void CClientDlg::ShowImage(const Mat& src, const UINT& ID)
{
	Mat dest = src.clone();
	if (src.empty())
	{
		AfxMessageBox(_T("没有获取图像"));
		return;
	}
	dest = cv::Scalar::all(0);
	// 计算将图片缩放到 Image 区域所需的比例因子
	double wRatio = dest.cols / (double)src.cols;
	double hRatio = dest.rows / (double)src.rows;
	double srcWH = src.cols / (double)src.rows;
	double desWH = dest.cols / (double)dest.rows;
	double scale = srcWH > desWH ? wRatio : hRatio;

	// 缩放后图片的宽和高
	int nw = (int)(src.cols * scale);
	int nh = (int)(src.rows * scale);

	// 为了将缩放后的图片存入 des 的正中部位，需计算图片在 des 左上角的期望坐标值
	int tlx = (int)((dest.cols - nw) / 2);
	int tly = (int)((dest.rows - nh) / 2);

	// 设置 des 的 ROI 区域，用来存入图片 img
	cv::Mat desRoi = dest(cv::Rect(tlx, tly, nw, nh));

	// 如果src是单通道图像，则转换为三通道图像
	if (src.channels() == 1)
	{
		cv::Mat src_c;
		cvtColor(src, src_c, CV_GRAY2BGR);
		// 对图片 src_t 进行缩放，并存入到 des 中
		resize(src_c, desRoi, desRoi.size());
	}
	else
	{
		// 对图片 src 进行缩放，并存入到 des 中
		resize(src, desRoi, desRoi.size());
	}
	CDC* pDC = GetDlgItem(ID)->GetDC();		// 获得显示控件的 DC
	HDC hDC = pDC->GetSafeHdc();				// 获取 HDC(设备句柄) 来进行绘图操作
	CRect rect;
	GetDlgItem(ID)->GetClientRect(&rect);	// 获取控件尺寸位置
	CvvImage image;
	IplImage cpy = desRoi;  //desRoi 是缩放后的图片，des是没有适应控件的图片
	image.CopyOf(&cpy);						// 复制图片
	image.DrawToHDC(hDC, &rect);				// 将图片绘制到显示控件的指定区域内
	ReleaseDC(pDC);/**/
}
//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
void CClientDlg::RecordTime(vector<CTimeRecord> &timeRecordList,const CString& timeComment)
{
	static CTimeRecord tempTimeRecord;
	tempTimeRecord.timeClock = clock();
	tempTimeRecord.timeComment = timeComment;
	timeRecordList.push_back(tempTimeRecord);
}
void CClientDlg::ShowTimeRecord(const vector<CTimeRecord>& timeRecordList)
{
	UpdateData(TRUE);
	if (timeRecordList.size() > 1)
	{
		char chTime[10];
		m_edtInfoShow = _T(""); //m_edtTimeRecord是信息显示编辑框变量
		for (int i = 1; i < timeRecordList.size(); i++)
		{
			m_edtInfoShow += timeRecordList[i].timeComment + _T(":");
			sprintf_s(chTime, "%dms", (timeRecordList[i].timeClock - timeRecordList[i - 1].timeClock));
			m_edtInfoShow += chTime;
			m_edtInfoShow += _T("\r\n");
		}
		m_edtInfoShow += _T("总时间：");
		int index = timeRecordList.size() - 1;
		sprintf_s(chTime, "%dms", timeRecordList[index].timeClock - timeRecordList[0].timeClock);
		m_edtInfoShow += chTime;
	}
	UpdateData(FALSE);
}
HCURSOR CClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CClientDlg::OnBnClickedBtnopen()
{
	// TODO:  在此添加控件通知处理程序代码
    CString tempStr;
	GetDlgItem(IDC_BtnOpen)->GetWindowText(tempStr);
	if (tempStr == _T("打开相机"))
	{
		m_capture.open(1);
		if (!m_capture.isOpened())
		{
			m_capture.open(0);
			if (!m_capture.isOpened())
			{
				MessageBox(_T("Failed to Open"));
				return;
			}
			else
			{
				m_capture.read(m_image);
				ShowImage(m_image, IDC_PicSrc);

				if (!m_imageGray.empty()) m_imageGray.release();
				m_imageGray.create(Size(m_image.cols, m_image.rows), CV_8UC1);
			
				GetDlgItem(IDC_BtnPause)->EnableWindow(TRUE);
				GetDlgItem(IDC_BtnPause)->SetWindowText(_T("暂停"));
				GetDlgItem(IDC_BtnOpen)->SetWindowText(_T("关闭相机"));
				SetTimer(1, 40, NULL);
			}
		}
	}
	else
	{
		if (m_capture.isOpened())
		{
			m_capture.release();
			GetDlgItem(IDC_BtnOpen)->SetWindowText(_T("打开相机"));
			KillTimer(1);
		}
	}
}


void CClientDlg::OnBnClickedBtnpause()
{
	// TODO:  在此添加控件通知处理程序代码
	CString tempStr;
	GetDlgItem(IDC_BtnPause)->GetWindowText(tempStr);
	if (tempStr == _T("播放"))
	{
		GetDlgItem(IDC_BtnPause)->SetWindowText(_T("暂停"));
		SetTimer(1, 40, NULL);
	}
	else
	{
		GetDlgItem(IDC_BtnPause)->SetWindowText(_T("播放"));
		KillTimer(1);
	}
}

Mat CClientDlg::ImageMergeCols(const Mat& img1, const Mat& img2)
{
	Mat img_merge(img1.rows, img1.cols + img2.cols, img1.type());
	Mat subMat = img_merge.colRange(0, img1.cols);
	img1.copyTo(subMat);
	subMat = img_merge.colRange(img1.cols, img1.cols + img2.cols);
	img2.copyTo(subMat);
	return img_merge;
}
void CClientDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	m_timeRecordList.clear();
	RecordTime(m_timeRecordList, _T("timeStart"));
	if (m_capture.isOpened())
	{
		if (m_capture.read(m_image))
		{
			ShowImage(m_image, IDC_PicSrc);

			if (m_prevImage.empty()) m_prevImage = m_image.clone();
			cvtColor(m_image, m_imageGray, CV_BGR2GRAY);
			if (m_prevGray.empty()) m_prevGray = m_imageGray.clone();
			RecordTime(m_timeRecordList, _T("图像灰度化"));
			m_imageMerge = ImageMergeCols(m_prevImage, m_image);
			RecordTime(m_timeRecordList, _T("彩色图合并"));
			m_faceAnalyze.tempImageMerge = m_imageMerge;

			m_faceAnalyze.FaceDetect(m_imageGray, m_faceAnalyze.face_cascade, m_faceAnalyze.m_facesBox);
			//m_faceAnalyze.FaceDetect(m_imageGray, m_faceAnalyze.m_facesBox);
			RecordTime(m_timeRecordList, _T("人脸检测"));

			m_faceAnalyze.FaceTracking(m_prevGray, m_imageGray, m_faceAnalyze.m_facesBox, m_faceAnalyze.m_humanFace);
			
			m_faceAnalyze.FacialOrgansDetect(m_imageGray, m_faceAnalyze.m_humanFace,m_imageMerge);
		    
			ShowRunResult();
			RecordTime(m_timeRecordList, _T("运行结果显示"));
			cv::swap(m_prevGray, m_imageGray);
			cv::swap(m_prevImage, m_image);
		}
	}
	
	ShowTimeRecord(m_timeRecordList);
	CDialogEx::OnTimer(nIDEvent);
}
void CClientDlg::ShowRunResult()
{
	for (int i = 0; i < m_faceAnalyze.m_facesBox.size(); i++)
	{
		//Rect tempRect = m_faceAnalyze.m_facesBox[i];
		//cv::circle(m_imageMerge, Point(tempRect.x + tempRect.width/2, tempRect.y + tempRect.height/2), 3, Scalar(255, 255, 0), 2);
		cv::rectangle(m_imageMerge, m_faceAnalyze.m_facesBox[i], Scalar(0, 0, 255), 1);
	}
	
	m_faceAnalyze.DrawFaceTrajectory(m_image, m_faceAnalyze.m_humanFace);

	for (int i = 0; i < m_faceAnalyze.m_humanFace.size(); i++)
	{
		Rect tempRect = m_faceAnalyze.m_humanFace[i].faceBox;
		tempRect.x -= (tempRect.width >> 4);
		tempRect.width += (tempRect.width >> 3);
		tempRect.y -= (tempRect.height >> 3);
		tempRect.height += (tempRect.height >> 2);
		if (tempRect.x>0 && tempRect.y > 0 && (tempRect.x + tempRect.width) < m_imageGray.cols && (tempRect.y + tempRect.height) < m_imageGray.rows)
		{
			cv::rectangle(m_imageMerge, m_faceAnalyze.m_humanFace[i].faceBox, Scalar(0, 255, 255), 2);
		}
		else
		{
			tempRect = m_faceAnalyze.m_humanFace[i].faceBox;
			cv::rectangle(m_imageMerge, m_faceAnalyze.m_humanFace[i].faceBox, Scalar(0, 255, 255), 2);
		}
		if (m_faceAnalyze.m_humanFace[i].bConfirmFace)
		{
			if (m_faceAnalyze.m_humanFace[i].bEyesDetected)
			{
				double dy = (m_faceAnalyze.m_humanFace[i].rightEyePoint.y - m_faceAnalyze.m_humanFace[i].leftEyePoint.y);
				double dx = (m_faceAnalyze.m_humanFace[i].rightEyePoint.x - m_faceAnalyze.m_humanFace[i].leftEyePoint.x);
				double radius = sqrt(dx*dx + dy*dy) / 4;
				circle(m_image, Point(m_faceAnalyze.m_humanFace[i].leftEyePoint.x, m_faceAnalyze.m_humanFace[i].leftEyePoint.y), radius, Scalar(255, 255, 0), 2);
				circle(m_image, Point(m_faceAnalyze.m_humanFace[i].rightEyePoint.x, m_faceAnalyze.m_humanFace[i].rightEyePoint.y), radius, Scalar(255, 255, 0), 2);
			}
			if (m_faceAnalyze.m_humanFace[i].bNoseDetected)
			{
				Rect noseRect = m_faceAnalyze.m_humanFace[i].noseRect;
				Point pts[1][4];
				pts[0][0] = Point(noseRect.x, noseRect.y + noseRect.height);
				pts[0][1] = Point(noseRect.x + noseRect.width, noseRect.y + noseRect.height);
				pts[0][2] = Point(noseRect.x + noseRect.width *0.75, noseRect.y);
				pts[0][3] = Point(noseRect.x + noseRect.width *0.25, noseRect.y);
				const Point* pt[1] = { pts[0] };
				int nPt[1] = { 4 };
				polylines(m_imageMerge, pt, nPt, 2,2,Scalar(255, 255, 0),2);
			}
			if (m_faceAnalyze.m_humanFace[i].bMouthDetected)
			{
				rectangle(m_imageMerge, m_faceAnalyze.m_humanFace[i].mouthRect, Scalar(255, 255, 0), 2);
			}
			for (int k = 0; k < m_faceAnalyze.m_humanFace[i].featPoints[1].size(); k++)
			{
				//circle(m_imageMerge, m_faceAnalyze.m_humanFace[i].featPoints[1][k], 2, Scalar(0, 255, 0), 1);
			}
			m_faceAnalyze.DrawLKMatch(m_imageMerge, m_faceAnalyze.m_humanFace[i].featTrajectoies);
		}
	}

	ShowImage(m_imageMerge, IDC_PicDest);
}
