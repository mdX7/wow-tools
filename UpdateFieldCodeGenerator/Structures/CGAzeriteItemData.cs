﻿namespace UpdateFieldCodeGenerator.Structures
{
    [HasChangesMask]
    public class CGAzeriteItemData
    {
        public static readonly ObjectType ObjectType = ObjectType.AzeriteItem;

        public static readonly UpdateField m_xp = new UpdateField(typeof(ulong), UpdateFieldFlag.Owner);
        public static readonly UpdateField m_level = new UpdateField(typeof(uint), UpdateFieldFlag.Owner);
        public static readonly UpdateField m_auraLevel = new UpdateField(typeof(uint), UpdateFieldFlag.Owner);
        public static readonly UpdateField m_knowledgeLevel = new UpdateField(typeof(uint), UpdateFieldFlag.Owner);
        public static readonly UpdateField m_DEBUGknowledgeWeek = new UpdateField(typeof(int), UpdateFieldFlag.Owner);
    }
}
