USE [RZ_GAMEDB]
GO
/****** Object:  Table [dbo].[RZ_INVEN]    Script Date: 12/25/2013 14:47:50 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[RZ_INVEN](
	[WORLD_ID] [int] NOT NULL,
	[ACCN_ID] [bigint] NULL,
	[OWNER_ID] [bigint] NOT NULL,
	[SLOT_TYPE] [int] NOT NULL,
	[SLOT_ID] [int] NOT NULL,
	[IUID] [bigint] NOT NULL,
	[ITEM_ID] [int] NULL,
	[STACK_AMT] [smallint] NOT NULL,
	[SOUL_COUNT] [tinyint] NOT NULL,
	[DURA] [tinyint] NOT NULL,
	[MAX_DURA] [tinyint] NOT NULL,
	[COLOR] [int] NOT NULL,
	[CLAIMED] [tinyint] NULL,
	[CHAR_PTM] [int] NULL,
	[PERIOD] [tinyint] NOT NULL,
	[USAGE_PERIOD] [int] NOT NULL,
	[EFF_START_DATE] [datetime] NULL,
	[EFF_END_DATE] [datetime] NULL,
	[REG_DATE] [datetime] NULL,
	[STORAGE_REG_DATE] [datetime] NULL,
	[INVEN_SN] [bigint] IDENTITY(1,1) NOT NULL,
	[CUR_XP] [int] NOT NULL,
	[NEXT_ATTUNE_XP] [int] NOT NULL,
 CONSTRAINT [PK_RZ_INVEN_OWNER_ID_SLOT_TYPE_SLOT_ID] PRIMARY KEY NONCLUSTERED 
(
	[OWNER_ID] ASC,
	[SLOT_TYPE] ASC,
	[SLOT_ID] ASC
)WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
EXEC sys.sp_addextendedproperty @name=N'COMMENTS', @value=N'??? ??? ?? ????.' , @level0type=N'SCHEMA',@level0name=N'dbo', @level1type=N'TABLE',@level1name=N'RZ_INVEN', @level2type=N'COLUMN',@level2name=N'ACCN_ID'
GO
EXEC sys.sp_addextendedproperty @name=N'COMMENTS', @value=N'??? ??? ??(???, ??).' , @level0type=N'SCHEMA',@level0name=N'dbo', @level1type=N'TABLE',@level1name=N'RZ_INVEN', @level2type=N'COLUMN',@level2name=N'OWNER_ID'
GO
EXEC sys.sp_addextendedproperty @name=N'COMMENTS', @value=N'?? ??. 1:?????, 2:?????, 3:??????, 4:?????.' , @level0type=N'SCHEMA',@level0name=N'dbo', @level1type=N'TABLE',@level1name=N'RZ_INVEN', @level2type=N'COLUMN',@level2name=N'SLOT_TYPE'
GO
EXEC sys.sp_addextendedproperty @name=N'COMMENTS', @value=N'?? ID.' , @level0type=N'SCHEMA',@level0name=N'dbo', @level1type=N'TABLE',@level1name=N'RZ_INVEN', @level2type=N'COLUMN',@level2name=N'SLOT_ID'
GO
EXEC sys.sp_addextendedproperty @name=N'COMMENTS', @value=N'??? ?? ????. ??? ???? 0?? ???.' , @level0type=N'SCHEMA',@level0name=N'dbo', @level1type=N'TABLE',@level1name=N'RZ_INVEN', @level2type=N'COLUMN',@level2name=N'IUID'
GO
EXEC sys.sp_addextendedproperty @name=N'COMMENTS', @value=N'??? ID. ??? ???? NULL?? ???.' , @level0type=N'SCHEMA',@level0name=N'dbo', @level1type=N'TABLE',@level1name=N'RZ_INVEN', @level2type=N'COLUMN',@level2name=N'ITEM_ID'
GO
EXEC sys.sp_addextendedproperty @name=N'COMMENTS', @value=N'??? ??.' , @level0type=N'SCHEMA',@level0name=N'dbo', @level1type=N'TABLE',@level1name=N'RZ_INVEN', @level2type=N'COLUMN',@level2name=N'STACK_AMT'
GO
EXEC sys.sp_addextendedproperty @name=N'COMMENTS', @value=N'??? ?? ?.' , @level0type=N'SCHEMA',@level0name=N'dbo', @level1type=N'TABLE',@level1name=N'RZ_INVEN', @level2type=N'COLUMN',@level2name=N'SOUL_COUNT'
GO
EXEC sys.sp_addextendedproperty @name=N'COMMENTS', @value=N'?? ???.' , @level0type=N'SCHEMA',@level0name=N'dbo', @level1type=N'TABLE',@level1name=N'RZ_INVEN', @level2type=N'COLUMN',@level2name=N'DURA'
GO
EXEC sys.sp_addextendedproperty @name=N'COMMENTS', @value=N'??? ??? ? ?? ?? ???.' , @level0type=N'SCHEMA',@level0name=N'dbo', @level1type=N'TABLE',@level1name=N'RZ_INVEN', @level2type=N'COLUMN',@level2name=N'MAX_DURA'
GO
EXEC sys.sp_addextendedproperty @name=N'COMMENTS', @value=N'??? ?.' , @level0type=N'SCHEMA',@level0name=N'dbo', @level1type=N'TABLE',@level1name=N'RZ_INVEN', @level2type=N'COLUMN',@level2name=N'COLOR'
GO
EXEC sys.sp_addextendedproperty @name=N'COMMENTS', @value=N'?? ??. 0:??????, 1:???.' , @level0type=N'SCHEMA',@level0name=N'dbo', @level1type=N'TABLE',@level1name=N'RZ_INVEN', @level2type=N'COLUMN',@level2name=N'CLAIMED'
GO
EXEC sys.sp_addextendedproperty @name=N'COMMENTS', @value=N'?? ??? ??? ??? ??' , @level0type=N'SCHEMA',@level0name=N'dbo', @level1type=N'TABLE',@level1name=N'RZ_INVEN', @level2type=N'COLUMN',@level2name=N'CHAR_PTM'
GO
EXEC sys.sp_addextendedproperty @name=N'COMMENTS', @value=N'??? ??? ??. 0:???, 1:???.' , @level0type=N'SCHEMA',@level0name=N'dbo', @level1type=N'TABLE',@level1name=N'RZ_INVEN', @level2type=N'COLUMN',@level2name=N'PERIOD'
GO
EXEC sys.sp_addextendedproperty @name=N'COMMENTS', @value=N'?? ?? ??(???).' , @level0type=N'SCHEMA',@level0name=N'dbo', @level1type=N'TABLE',@level1name=N'RZ_INVEN', @level2type=N'COLUMN',@level2name=N'USAGE_PERIOD'
GO
EXEC sys.sp_addextendedproperty @name=N'COMMENTS', @value=N'?? ??(?? ??)  ??.' , @level0type=N'SCHEMA',@level0name=N'dbo', @level1type=N'TABLE',@level1name=N'RZ_INVEN', @level2type=N'COLUMN',@level2name=N'EFF_START_DATE'
GO
EXEC sys.sp_addextendedproperty @name=N'COMMENTS', @value=N'?? ??(?? ??)  ??.' , @level0type=N'SCHEMA',@level0name=N'dbo', @level1type=N'TABLE',@level1name=N'RZ_INVEN', @level2type=N'COLUMN',@level2name=N'EFF_END_DATE'
GO
EXEC sys.sp_addextendedproperty @name=N'COMMENTS', @value=N'??? ?? ??.' , @level0type=N'SCHEMA',@level0name=N'dbo', @level1type=N'TABLE',@level1name=N'RZ_INVEN', @level2type=N'COLUMN',@level2name=N'REG_DATE'
GO
EXEC sys.sp_addextendedproperty @name=N'COMMENTS', @value=N'???? ??? ??.' , @level0type=N'SCHEMA',@level0name=N'dbo', @level1type=N'TABLE',@level1name=N'RZ_INVEN', @level2type=N'COLUMN',@level2name=N'STORAGE_REG_DATE'
GO
EXEC sys.sp_addextendedproperty @name=N'COMMENTS', @value=N'??????. ??? ???? ???? ??..' , @level0type=N'SCHEMA',@level0name=N'dbo', @level1type=N'TABLE',@level1name=N'RZ_INVEN', @level2type=N'COLUMN',@level2name=N'INVEN_SN'
GO
EXEC sys.sp_addextendedproperty @name=N'COMMENTS', @value=N'???? ??? ??? ??? ?? ???.' , @level0type=N'SCHEMA',@level0name=N'dbo', @level1type=N'TABLE',@level1name=N'RZ_INVEN'
GO
ALTER TABLE [dbo].[RZ_INVEN]  WITH CHECK ADD  CONSTRAINT [CK_RZ_INVEN_SLOT_TYPE_RANGE] CHECK  (([SLOT_TYPE]=(4) OR [SLOT_TYPE]=(3) OR [SLOT_TYPE]=(2) OR [SLOT_TYPE]=(1)))
GO
ALTER TABLE [dbo].[RZ_INVEN] CHECK CONSTRAINT [CK_RZ_INVEN_SLOT_TYPE_RANGE]
GO
ALTER TABLE [dbo].[RZ_INVEN]  WITH CHECK ADD  CONSTRAINT [CK_RZ_INVEN_STACK_AMT_RANGE] CHECK  (([STACK_AMT]>=(0) AND [STACK_AMT]<=(999)))
GO
ALTER TABLE [dbo].[RZ_INVEN] CHECK CONSTRAINT [CK_RZ_INVEN_STACK_AMT_RANGE]
GO
ALTER TABLE [dbo].[RZ_INVEN] ADD  DEFAULT ((0)) FOR [IUID]
GO
ALTER TABLE [dbo].[RZ_INVEN] ADD  DEFAULT ((0)) FOR [STACK_AMT]
GO
ALTER TABLE [dbo].[RZ_INVEN] ADD  DEFAULT ((0)) FOR [SOUL_COUNT]
GO
ALTER TABLE [dbo].[RZ_INVEN] ADD  DEFAULT ((0)) FOR [DURA]
GO
ALTER TABLE [dbo].[RZ_INVEN] ADD  DEFAULT ((0)) FOR [MAX_DURA]
GO
ALTER TABLE [dbo].[RZ_INVEN] ADD  DEFAULT ((0)) FOR [COLOR]
GO
ALTER TABLE [dbo].[RZ_INVEN] ADD  DEFAULT ((0)) FOR [PERIOD]
GO
ALTER TABLE [dbo].[RZ_INVEN] ADD  DEFAULT ((0)) FOR [USAGE_PERIOD]
GO
ALTER TABLE [dbo].[RZ_INVEN] ADD  CONSTRAINT [DF_RZ_INVEN_CUR_XP]  DEFAULT ((-1)) FOR [CUR_XP]
GO
ALTER TABLE [dbo].[RZ_INVEN] ADD  CONSTRAINT [DF_RZ_INVEN_NEXT_ATTUNE_XP]  DEFAULT ((-1)) FOR [NEXT_ATTUNE_XP]
GO
