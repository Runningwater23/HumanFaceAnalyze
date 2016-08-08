
// ClientDlg.h : 头文件
//

#pragma once
#include "opencv2/opencv.hpp"
#include "CvvImage.h"
using namespace cv;

#include <vector>
using namespace std;

#include "FaceAnalyze.h"
// CClientDlg 对话框
class CClientDlg : public CDialogEx
{
public:
	struct CTimeRecord
	{
		int timeClock;
		CString timeComment;
	};
public:
	vector<CTimeRecord> m_timeRecordList;

	VideoCapture m_capture;
	Mat m_image;
	Mat m_prevImage;
	Mat m_imageGray;
	Mat m_prevGray;
	Mat m_imageMerge;

	CFaceAnalyze m_faceAnalyze;
public:
	void ShowImage(const Mat& src, const UINT& ID);
	void RecordTime(vector<CTimeRecord> &timeRecordList,const CString& timeComment);
	void ShowTimeRecord(const vector<CTimeRecord>& timeRecordList);
	Mat ImageMergeCols(const Mat& img1, const Mat& img2);
	void ShowRunResult();
// 构造
public:
	CClientDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_CLIENT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnopen();
	afx_msg void OnBnClickedBtnpause();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CString m_edtInfoShow;
};
