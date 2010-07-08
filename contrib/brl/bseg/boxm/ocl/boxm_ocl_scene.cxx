#include "boxm_ocl_scene.h"
#include <vcl_iostream.h>
#include <vcl_string.h>
#include <vcl_sstream.h>
#include <vcl_fstream.h>
#include <boxm/boxm_apm_traits.h>
#include <vcl_cstdlib.h> // for rand()
/* xml includes */
#include <vsl/vsl_basic_xml_element.h>
#include <vgl/xio/vgl_xio_point_3d.h>
#include <vgl/xio/vgl_xio_vector_3d.h>

/* io includes */
#include <vnl/io/vnl_io_vector_fixed.h>
#include <vbl/io/vbl_io_array_1d.h>
#include <vbl/io/vbl_io_array_2d.h>
#include <vbl/io/vbl_io_array_3d.h>

#define BLOCK_NAME "blocks.bin"
#define DATA_NAME "data.bin"

boxm_ocl_scene::boxm_ocl_scene(vbl_array_3d<int4> blocks, 
                               vbl_array_2d<int4> tree_buffers, 
                               vbl_array_2d<float16> data_buffers, 
                               vbl_array_1d<int2> mem_ptrs, 
                               bgeo_lvcs lvcs,
                               vgl_point_3d<double> origin,
                               vgl_vector_3d<double> block_dim)
{

  //copy all blocks 
  blocks_ = vbl_array_3d<int4>(blocks);
  tree_buffers_ = vbl_array_2d<int4>(tree_buffers); 
  data_buffers_ = vbl_array_2d<float16>(data_buffers);
  mem_ptrs_ = vbl_array_1d<int2>(mem_ptrs), 
  
  num_tree_buffers_ = (int) tree_buffers.rows();
  tree_buff_length_ = (int) tree_buffers.cols();
  lvcs_ = lvcs;
  origin_ = origin;
  block_dim_ = block_dim;
}
 


//intializes Scene from XML file
boxm_ocl_scene::boxm_ocl_scene(vcl_string filename)
{
  //load the scene xml file
  this->load_scene(filename);
}

//: Saves XML scene file, block binary and data binary files to 'dir' 
bool boxm_ocl_scene::save_scene(vcl_string dir)
{
  vcl_cout<<"boxm_ocl_scene::save_scene to "<<dir<<vcl_endl;
  //get the paths straight... 
  vcl_string block_path = dir + "blocks.bin";
  vcl_string data_path = dir + "data.bin";
  vcl_string xml_path = dir + "scene.xml";

  //write out to XML file
//  vcl_ofstream xmlstrm(xml_path.c_str());
//  boxm_ocl_scene scene = boxm_ocl_scene(this);
//  x_write(xmlstrm, scene, "scene");
  
  //write to block binary file
  vsl_b_ofstream bin_os(block_path);
  if(!bin_os) {
    vcl_cout<<"cannot open "<<block_path<<" for writing";
    return false;
  }
  vsl_b_write(bin_os, num_tree_buffers_);
  vsl_b_write(bin_os, tree_buff_length_);
  vsl_b_write(bin_os, tree_buffers_);
  vsl_b_write(bin_os, blocks_);
  vsl_b_write(bin_os, mem_ptrs_);
  bin_os.close();
  
  //write to data binary file 
  vsl_b_ofstream dat_os(data_path);
  if(!dat_os) {
    vcl_cout<<"cannot open "<<data_path<<" for writing";
    return false;
  }
  vsl_b_write(dat_os, data_buffers_);
  dat_os.close();

  return true;
}




//:Loads small block scene from XML file (filename is xml file path)
//first reads XML file and stores model information
//then creates blocks and reads 
//this needs to initialize: blocks_, tree_buffers_, data_buffers_ and mem_ptrs_
bool boxm_ocl_scene::load_scene(vcl_string filename)
{

  /* parser_ stores all the xml information */
  xml_path_ = filename;
  if (filename.size() > 0) {
    vcl_FILE* xmlFile = vcl_fopen(filename.c_str(), "r");
    if (!xmlFile){
    vcl_cerr << filename.c_str() << " error on opening\n";
    return false;
    }
    if (!parser_.parseFile(xmlFile)) {
      vcl_cerr << XML_ErrorString(parser_.XML_GetErrorCode()) << " at line "
               << parser_.XML_GetCurrentLineNumber() << '\n';
      return false;
    }
  }
  
  /* store world information */
  parser_.lvcs(lvcs_);
  origin_ = parser_.origin();
  rpc_origin_ = parser_.origin();
  block_dim_ = parser_.block_dim();
  
  /* load tree binary information */
  bool tree_loaded = this->init_existing_scene(); 
  if(tree_loaded) {
    vcl_cout<<"existing scene initialized "<<vcl_endl;
    bool data_loaded = this->init_existing_data();
    if(!data_loaded) {
      //init empty data to fit the tree structure
      vcl_cout<<"data.bin not found, initializing empty data"<<vcl_endl;
      this->init_empty_data(); 
    }
  }
  else {
    vcl_cout<<"tree_not loaded, initializing empty scene"<<vcl_endl;
    parser_.tree_buffer_shape(num_tree_buffers_, tree_buff_length_);
    this->init_empty_scene();
  }
  
  return true;
}

//initializes existing scene given the parser's paths
bool boxm_ocl_scene::init_existing_scene() 
{
  vcl_string dir, pref;
  parser_.paths(dir, pref);
  vcl_string block_path = dir + "/blocks.bin";
  vsl_b_ifstream is(block_path, vcl_ios_binary);
  if(!is) {
    vcl_cout<<"init_existing_scene: file "<<block_path<<" doesn't exist"<<vcl_endl;
    return false;
  }
  vsl_b_read(is, num_tree_buffers_);
  vsl_b_read(is, tree_buff_length_);
  vsl_b_read(is, tree_buffers_);
  vsl_b_read(is, blocks_);
  vsl_b_read(is, mem_ptrs_);
  return true;
}

//initializes existing data given the parser's paths
bool boxm_ocl_scene::init_existing_data()
{
  vcl_string dir, pref;
  parser_.paths(dir, pref);
  vcl_string data_path = dir + "/data.bin";
  vsl_b_ifstream is(data_path, vcl_ios_binary);
  if(!is) {
    vcl_cout<<"init_existing_data: file "<<data_path<<" doesn't exist"<<vcl_endl;
    return false;
  }
  vsl_b_read(is, data_buffers_);
  is.close();
  return true;
}


//initializes empty scene with specifications from xml file
bool boxm_ocl_scene::init_empty_scene()
{
  vcl_cout<<"initializing empty scene ... "<<vcl_endl;
  //tree buffers 
  int4 init_cell(-1);
  tree_buffers_ = vbl_array_2d<int4>(num_tree_buffers_, tree_buff_length_, init_cell);

  //initialize 3d block structure, init to -1
  int4 init_blk(-1);
  vgl_vector_3d<unsigned> blk_nums = parser_.block_nums();
  blocks_ =  vbl_array_3d<int4>(blk_nums.x(), blk_nums.y(), blk_nums.z(), init_blk);
  
  //initialize book keeping mem ptrs
  int2 init_mem(0);
  mem_ptrs_ = vbl_array_1d<int2>(num_tree_buffers_, init_mem);
  
  //initialize data buffers
  float16 init_dat(0.0f);
  data_buffers_ =  vbl_array_2d<float16>(num_tree_buffers_, tree_buff_length_, init_dat);

  //now assign block ptrs to things
  //for each block[i,j,k], throw it's root tree randomly into one of the tree buffers
  for(int i=0; i<blk_nums.x(); i++) {
    for(int j=0; j<blk_nums.y(); j++) {
      for(int k=0; k<blk_nums.z(); k++) {
        
        //randomly choose a buffer, and get the first free spot in memory
        int buffIndex = (int) (num_tree_buffers_-1)*(vcl_rand()/(RAND_MAX+1.0));
        int buffOffset = mem_ptrs_[buffIndex][1];
        int4 blk(0);
        blk[0] = buffIndex;  //buffer index 
        blk[1] = buffOffset; //buffer offset to root 
        blk[3] = 1;          //tree has size of 1 now
        blk[4] = 0;          //nothign for now
        blocks_[i][j][k] = blk;
        
        //put root in that memory
        int4 root; 
        root[0] = -1; //no parent for root
        root[1] = -1; // no children yet
        root[2] = buffOffset; // points to datum
        root[4] = 0;  // not used yet
        tree_buffers_[buffIndex][buffOffset] = root;
        
        //put data in memory 
        float16 datum(0.0f);
        datum[0] = .1;
        data_buffers_[buffIndex][buffOffset] = datum;
        
        //make sure mem spot is now taken
        mem_ptrs_[buffIndex][1]++;
      }
    }
  }
  
  return true;
}
/* initializes scene data assuming that the tree structure has already been initalized */
bool boxm_ocl_scene::init_empty_data()
{
  //TODO implement me and work me into init_empty_scene();
}



/******************************** XML WRITE **************************/
void x_write(vcl_ostream &os, boxm_ocl_scene& scene, vcl_string name)
{
  boxm_scene_parser info = scene.parser();

  //open root tag
  vsl_basic_xml_element scene_elm(name);
  scene_elm.x_write_open(os);

  //write appearance model
  vsl_basic_xml_element app_model(APP_MODEL_TAG);
  boxm_apm_type apm = boxm_apm_types::str_to_enum(info.app_model().data());
  app_model.add_attribute("type", boxm_apm_types::app_model_strings[apm]);
  app_model.x_write(os);

  /* next four are not really necessary... */
  //write multi bin boolean
  vsl_basic_xml_element bin(MULTI_BIN_TAG);
  bin.add_attribute("value", info.multi_bin()? 1 : 0);
  bin.x_write(os);
  //write save internal nodes boolean
  vsl_basic_xml_element save_nodes(SAVE_INTERNAL_NODES_TAG);
  save_nodes.add_attribute("value", info.save_internal_nodes()? 1 : 0);
  save_nodes.x_write(os);
  //write save platform independent boolean
  vsl_basic_xml_element save_platform_independent(SAVE_PLATFORM_INDEPENDENT_TAG);
  save_platform_independent.add_attribute("value", info.save_platform_independent()? 1 : 0);
  save_platform_independent.x_write(os);
  //write load_all_blocks boolean 
  vsl_basic_xml_element load_all_blocks(LOAD_ALL_BLOCKS_TAG);
  load_all_blocks.add_attribute("value", info.load_all_blocks()? 1 : 0);
  load_all_blocks.x_write(os);
  
  //write lvcs information 
  bgeo_lvcs lvcs; 
  info.lvcs(lvcs);
  lvcs.x_write(os, LVCS_TAG);
  vgl_point_3d<double> test;
  x_write(os, test, LOCAL_ORIGIN_TAG);
  x_write(os, info.block_dim(), BLOCK_DIMENSIONS_TAG);

  //write block numbers for x,y,z 
  vsl_basic_xml_element blocks(BLOCK_NUM_TAG);
  int x_num, y_num, z_num;
  scene.block_num(x_num, y_num, z_num);
  blocks.add_attribute("x_dimension", x_num);
  blocks.add_attribute("y_dimension", y_num);
  blocks.add_attribute("z_dimension", z_num);
  blocks.x_write(os);
  
  //write block dimensions for each 
  vsl_basic_xml_element bnum(BLOCK_NUM_TAG);
  int x_dim, y_dim, z_dim;
  scene.block_dim(x_dim, y_dim, z_dim);
  blocks.add_attribute("x", x_dim);
  blocks.add_attribute("y", y_dim);
  blocks.add_attribute("z", z_dim);
  blocks.x_write(os);
  
  //write scene path for (needs to know where blocks are)
  vcl_string path, pref;
  info.paths(path, pref);
  vsl_basic_xml_element paths(SCENE_PATHS_TAG);
  paths.add_attribute("path", path);
  paths.add_attribute("block_prefix", pref);
  paths.x_write(os);
  
  //write octree levels tag
  unsigned max_level, init_level;
  info.levels(max_level, init_level);
  vsl_basic_xml_element tree(OCTREE_LEVELS_TAG);
  tree.add_attribute("max", (int) max_level);
  tree.add_attribute("init", (int) init_level);
  tree.x_write(os);
  
  scene_elm.x_write_close(os);
}

/********************* stream I/O *****************************/
vcl_ostream& operator <<(vcl_ostream &s, boxm_ocl_scene& scene)
{

  //get shape of tree buffers 
  int num, len;
  scene.tree_buffer_shape(num, len);
  
  //get block numbers in each dimension
  int x_num, y_num, z_num;
  scene.block_num(x_num, y_num, z_num);
 
  //get dimension of each block 
  int x_dim, y_dim, z_dim;
  scene.block_dim(x_dim, y_dim, z_dim);
  
  
  s <<"---OCL_SCENE--------------------------------" << vcl_endl
    <<"[block_nums "<<x_num<<","<<y_num<<","<<z_num<<"] "
    <<"[blk_dim "<<x_dim<<","<<y_dim<<","<<z_dim<<"] " << vcl_endl
    <<"[num_buffs " << num << "] " 
    <<"[buff_length " << len << "] " << vcl_endl;
    
    
  //print out buffer free space 
  typedef vnl_vector_fixed<int,2> int2;
  typedef vnl_vector_fixed<int,4> int4;
  vbl_array_2d<int4> tree_buffers = scene.tree_buffers();
  vbl_array_1d<int2> mem_ptrs = scene.mem_ptrs();
  s << "[free space: ";
  for(int i=0; i<mem_ptrs.size(); i++) {
    int start=mem_ptrs[i][0];
    int end = mem_ptrs[i][1];
    int freeSpace = (start >= end)? start-end : tree_buffers.cols() - (end-start);
    s <<"buff["<<i<<"]="<<freeSpace<<" blocks"<<", ";
  }
  s << vcl_endl;
  s <<"--------------------------------------------" << vcl_endl;
  return s;
  
  
  //print 3 corner blocks for now
//   //list all of the blocks 
//  
//  
//  //print out each tree 
//  for(int i=0; i<numBlocks; i++){
//    int blkRoot = block_ptrs[2*i];
//    int blkSize = block_ptrs[2*i+1];
//    vcl_cout<<"---- block "<<i<<" at "<<blkRoot<<"-----"<<vcl_endl;
//    for(int j=0; j<blkSize; j++){
//    
//      //print tree cell
//      vcl_cout<<"cell @ "<<j<<" (absolute: "<<j+blkRoot<<" : ";
//      for(int k=0; k<4; k++)
//        vcl_cout<<blocks[4*blkRoot+4*j+k]<<" ";

//      //print data if it exists
//      int data_ptr = blocks[4*blkRoot+4*j+2];
//      if(data_ptr >= 0) {
//        vcl_cout<<"  data @ "<<data_ptr<<" : ";
//        for(int k=0; k<16; k++) 
//          vcl_cout<<data[16*data_ptr+k]<<" ";          
//      }
//      else {
//        vcl_cout<<"  data for this cell not stored "; 
//      }
//      vcl_cout<<vcl_endl;
//    }
//  }


}



