
	DWORD CSkinRichEdit::GetSelectObjectUser(CString& strPathRet, CString strPicFlag)
	{
		DWORD dRet = 0;
		int nCount = m_pIRichEditOle->GetObjectCount(); // 当前m_ChatEdit中所有的object对象
		
		for ( int i=nCount -1; i >= 0; i-- )
		{
			REOBJECT reobject;
			memset( &reobject, 0 , sizeof(REOBJECT) );
			reobject.cbStruct = sizeof(REOBJECT);

			SCODE sc = m_pIRichEditOle->GetObject( i, &reobject, REO_GETOBJ_ALL_INTERFACES ); // 获取到reobject对象
			if ( sc != S_OK )
			{
				continue;
			}
			
			if ( (reobject.dwFlags & REO_SELECTED) != REO_SELECTED ) // 检验当前reobject是否处于选中状态
			{
				continue; // 不是当前选中的对象，继续查找
			}

			IDataObject* pDataObject = NULL;
			sc = reobject.poleobj->QueryInterface( IID_IDataObject, (void**)&pDataObject ); // 获取IDataObject信息
			if ( sc != S_OK )
				break;
			CString strPath = "";
			if(reobject.dwUser > 0)
			{			
				dRet = reobject.dwUser;
			}
			else
			{
				IEnumFORMATETC *pEnumFmt = NULL;

				if (S_OK != pDataObject->EnumFormatEtc(DATADIR_GET,&pEnumFmt))
					break;
				
				while(Ret == S_OK)
				{
					HRESULT ret=pEnumFmt->Next(1,&fm,&Fetched);
					if(SUCCEEDED(ret))
					{
						if( (fm.cfFormat == CF_METAFILEPICT	)//媒体
							||fm.cfFormat == CF_BITMAP	//位图
							|| fm.cfFormat == CF_DIB)	//DIB
						{
							if(fm.cfFormat == CF_DIB)
								fm.cfFormat = CF_BITMAP;
							if (S_OK != pDataObject->GetData(&fm, &sm))
							{
								break;
							}

							if (sm.pUnkForRelease != NULL)
							{
								break;
							}

							if (fm.cfFormat == CF_BITMAP	//位图
								|| fm.cfFormat == CF_DIB)
							{
								CImage image;
								image.Attach(sm.hBitmap);
								image.Save(strPath);
							}
							else
							{
								LPMETAFILEPICT pMFP = (LPMETAFILEPICT) GlobalLock (sm.hMetaFilePict);
								SaveToBitMap(pMFP, strPath);
								GlobalUnlock(sm.hMetaFilePict);
							}

							dRet = reobject.dwUser = SetImagePath(strPath);
							::ReleaseStgMedium(&sm);

							Ret = S_FALSE;

						}
					}
					else
					{
						Ret = S_FALSE;
					}
				}
			}
			pDataObject->Release();
			break;

		}

		if (dRet > 1000)
		{
			GetImagePath(dRet, strPathRet);		
		}

		//注意:reObject有Release()问题! 比如GetObject()方法之后, 具体请参见MSDN!!!!!!!!!!!!!!!!!!!!!!!!

		return dRet;
	}

	//////////////////////////////////////////////////////////////////////////



	void CSkinRichEdit::SaveToBitMap(LPMETAFILEPICT pMFP, CString strPath )
	{
		SIZE size;	
		CDC* pDC = GetDC();
		if (pDC)
		{
			size.cx = pMFP->xExt;
			size.cy = pMFP->yExt;
			pDC->HIMETRICtoDP(&size);

			CBitmap bm;
			bm.CreateCompatibleBitmap(pDC, abs(size.cx), abs(size.cy));
			CDC memdc;
			memdc.CreateCompatibleDC(pDC);
			CBitmap* pOldBitmap = memdc.SelectObject(&bm);
			memdc.SetMapMode(pMFP->mm) ;
			memdc.SetViewportExt(abs(size.cx), abs(size.cy)) ;
			memdc.SetViewportOrg(0, 0);
			memdc.PlayMetaFile(pMFP->hMF);
			CImage img;
			img.Attach((HBITMAP)bm.GetSafeHandle());
			img.Save(strPath);
			memdc.SelectObject(pOldBitmap);
			memdc.DeleteDC();
			bm.DeleteObject();
			ReleaseDC(pDC);
		}
	}