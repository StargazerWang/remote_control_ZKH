
Google ref rcu for TPV R52GES_T202 , Author :Telink Kidd


--------------------------------------------------------------------
ver v00.09_20210802
Update
1 init release for design house S
2 patch new zibit flash driver    

--------------------------------------------------------------------
ver v00.07_20210329 

Update:
1 PTM feature check macro for code pieces
	#define PTM                                   FALSE//TRUE // production test mode, anohter test mode other than MP_TEST_MODE
		#define PTM_FORCE_ENTER                   TRUE // force to enter production test mode, for quick test

--------------------------------------------------------------------
ver v00.05_20210323 

Update:
1 add back telink OTA, support both telink and google OTA, but not support 2 OTA to enable at the same time yet.

Bug fix:
1 update IO wake up poliraty when STUCK_KEY_PROCESS_ENABLE in blt_pm_proc()
--------------------------------------------------------------------
ver v00.03_20210317 

Note:
1 Don't change/remove OTA(and schedule OTA) related firmware APP layer logic 

NEW ADD:
1 initial release , full function follow software requirement, FTM not included

Bug fix:
later TBD

TODO:
1 Major
	NA
2 Minor
	NA

--------------------------------------------------------------------
