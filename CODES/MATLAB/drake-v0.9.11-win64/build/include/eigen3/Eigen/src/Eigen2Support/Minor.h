 �@�~��#�9      �@���F�8      �@���F�7      �@𬚼j~��6      �@�?|��5      �@�    �4      �@�=p���3      �@�~��#�2      �@𬋾vȴ�1      �@�=p���0      �@���F�/      �@𬇾vȴ�.      �@�    �-      �@�    �,      �@�|�i�+      �@�}?|��*      �@�y?|��)      �@�v~��#�(      �@�t�j~��'      �@�s?|��&      �@�r    �%      �@�p~��#�$      �@�n�j~��#      �@�k|�i�"      �@�h�j~���   SQLite format 3   @    �   �                                                           � .Z    � 
Y
	�	I�D�o�                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        j;�indexfschanges_name_hint_idxfschangesCREATE INDEX fschanges_name_hint_idx on fschanges (name_hint)� I�%indexfschanges_direction_action_idxfschangesCREATE INDEX fschanges_direction_action_idx on fschanges (direction, action)`5{indexfschanges_action_idxfschangesCREATE INDEX fschanges_action_idx on fschanges (action)g
9�indexfschanges_old_path_idxfschangesCREATE INDEX fschanges_old_path_idx on fschanges (old_path)j	;�indexfschanges_full_path_idxfschanges
CREATE INDEX fschanges_full_path_idx on fschanges (full_path)�Q�5indexfschanges_new_inode_new_volume_idxfschanges	CREATE INDEX fschanges_new_inode_new_volume_idx on fschanges (new_inode, new_volume)tA�indexfschanges_inode_volume_idxfschangesCREATE INDEX fschanges_inode_volume_idx on fschanges (inode, volume)`5{indexfschanges_doc_id_idxfschangesCREATE INDEX fschanges_doc_id_idx on fschanges (doc_id)Z1sindexfschanges_hash_idxfschangesCREATE INDEX fschanges_hash_idx on fschanges (hash)P++Ytablesqlite_sequencesqlite_sequenceCREATE TABLE sqlite_sequence(name,seq)�$�tablefschangesfschangesCREATE TABLE fschanges (identifier INTEGER, direction TEXT, action TEXT, inode INTEGER, volume TEXT, path TEXT, name TEXT, route TEXT, mapped_path TEXT, mapped_path_complete INTEGER, mapped_old_path TEXT, mapped_old_path_complete INTEGER, doc_id TEXT, parent_inode INTEGER, parent_volume TEXT, new_inode INTEGER, new_volu