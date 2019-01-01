extern box_info *get_box(int x,int y,int z,int denom);
extern void get_existing_faces(box_info *box);
extern void create_new_faces(box_info *box);
extern void get_existing_edges(box_info *box,face_info *face,int code);
extern void create_new_edges(face_info *face);
extern int get_sols_on_edge(edge_info *edge,sol_info *sols[2],int count);
extern int count_sols_on_edge(edge_info *edge);
extern sol_info *get_nth_sol_on_edge(edge_info *edge,int n);
extern int get_sols_on_face(face_info *face,sol_info *sols[2]);
extern int count_sols_on_face(face_info *face);
extern sol_info *get_nth_sol_on_face(face_info *face,int n);
extern int get_nodes_on_face(face_info *face,node_info *nodes[2],int count);
extern int count_nodes_on_face(face_info *face);
extern node_info *get_nth_node_on_face(face_info *face,int n);
extern int get_nodes_on_box_faces(box_info *box,node_info *nodes[2]);
extern node_info *get_nth_node_on_box(box_info *box,int n);
extern void split_face(face_info *face,face_info *face1,face_info *face2,
			face_info *face3,face_info *face4);
extern void distribute_nodes(face_info *face,face_info *face1,face_info *face2,
			face_info *face3,face_info *face4);
extern void split_box(box_info *box,box_info *lfd,box_info *rfd,box_info *lbd,box_info *rbd,
			box_info *lfu,box_info *rfu,box_info *lbu,box_info *rbu);

















