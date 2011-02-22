#if NVIDIA
 #pragma OPENCL EXTENSION cl_khr_gl_sharing : enable
#endif


#ifdef MOG_TYPE_16 
    #define MOG_TYPE int4
#endif
#ifdef MOG_TYPE_8 
    #define MOG_TYPE int2
#endif
#ifdef RENDER
//need to define a struct of type AuxArgs with auxiliary arguments
// to supplement cast ray args
typedef struct
{
  __global float* alpha; 
  __global MOG_TYPE*  mog;
  float* expint; 
} AuxArgs;  

//forward declare cast ray (so you can use it)
void cast_ray(int,int,float,float,float,float,float,float, 
              __constant RenderSceneInfo*, __global int4*, 
              __local uchar16*, __constant uchar *,__local uchar *, 
              float*, AuxArgs); 
__kernel
void
render_bit_scene( __constant  RenderSceneInfo    * linfo,
                  __global    int4               * tree_array,
                  __global    float              * alpha_array,
                  __global    MOG_TYPE           * mixture_array,
                  __global    float16            * camera,        // camera orign and SVD of inverse of camera matrix
                  __global    uint               * exp_image,      // input image and store vis_inf and pre_inf
                  __global    uint4              * exp_image_dims,
                  __global    float              * output,
                  __constant  uchar              * bit_lookup,
                  __global    float              * vis_image,
                  __local     uchar16            * local_tree,
                  __local     uchar              * cumsum,        //cumulative sum helper for data pointer
                  __local     int                * imIndex)
{
  //----------------------------------------------------------------------------
  //get local id (0-63 for an 8x8) of this patch + image coordinates and camera
  // check for validity before proceeding
  //----------------------------------------------------------------------------
  uchar llid = (uchar)(get_local_id(0) + get_local_size(0)*get_local_id(1));
  int i=0,j=0;
  i=get_global_id(0);
  j=get_global_id(1);

  // check to see if the thread corresponds to an actual pixel as in some
  // cases #of threads will be more than the pixels.
  if (i>=(*exp_image_dims).z || j>=(*exp_image_dims).w) 
    return;

  //----------------------------------------------------------------------------
  // Calculate ray origin, and direction
  // (make sure ray direction is never axis aligned)
  //----------------------------------------------------------------------------
  float ray_ox, ray_oy, ray_oz, ray_dx, ray_dy, ray_dz;
  calc_scene_ray(linfo, camera, i, j, &ray_ox, &ray_oy, &ray_oz, &ray_dx, &ray_dy, &ray_dz);  

  //----------------------------------------------------------------------------
  // we know i,j map to a point on the image, have calculated ray
  // BEGIN RAY TRACE
  //----------------------------------------------------------------------------
  //Store image index (may save a register).  Also initialize VIS and expected_int
  imIndex[llid] = j*get_global_size(0)+i;
  uint  eint    = as_uint(exp_image[imIndex[llid]]);
  uchar echar   = convert_uchar(eint);
  float expint  = convert_float(echar)/255.0f;
  float vis     = vis_image[imIndex[llid]];
  AuxArgs aux_args; 
  aux_args.alpha  = alpha_array; 
  aux_args.mog    = mixture_array;
  aux_args.expint = &expint;
  cast_ray( i, j,
            ray_ox, ray_oy, ray_oz,
            ray_dx, ray_dy, ray_dz,
            linfo, tree_array,                                    //scene info
            local_tree, bit_lookup, cumsum, &vis, aux_args);      //utility info
            
  //store the expected intensity (as UINT)
  exp_image[imIndex[llid]] =  rgbaFloatToInt((float4) expint); 

  //store visibility at the end of this block
  vis_image[imIndex[llid]] = vis;
}
#endif

#ifdef CHANGE
//need to define a struct of type AuxArgs with auxiliary arguments
// to supplement cast ray args
typedef struct
{
  __global float* alpha; 
  __global int2*  mog;
  float* expint; 
  float* intensity;
  float* exp_prob_int; 
} AuxArgs;  

//forward declare cast ray (so you can use it)
void cast_ray(int,int,float,float,float,float,float,float,constant RenderSceneInfo*,
              global int4*,local uchar16*,constant uchar*,local uchar*, float*, AuxArgs); 

__kernel
void
change_detection_bit_scene( __constant  RenderSceneInfo    * linfo,
                            __global    int4               * tree_array,
                            __global    float              * alpha_array,
                            __global    uchar8             * mixture_array,
                            __global    float16            * camera,        // camera orign and SVD of inverse of camera matrix
                            __global    float              * in_image,      // input image and store vis_inf and pre_inf
                            __global    uint               * exp_image,      // input image and store vis_inf and pre_inf
                            __global    uint               * prob_exp_image,       //input image
                            __global    uint               * change_image,      // input image and store vis_inf and pre_inf
                            __global    uint4              * exp_image_dims,
                            __global    float              * output,
                            __constant  uchar              * bit_lookup,
                            __global    float              * vis_image,
                            __local     uchar16            * local_tree,
                            __local     uchar              * cumsum,        //cumulative sum helper for data pointer
                            __local     int                * imIndex)
{
  //----------------------------------------------------------------------------
  //get local id (0-63 for an 8x8) of this patch + image coordinates and camera
  // check for validity before proceeding
  //----------------------------------------------------------------------------
  uchar llid = (uchar)(get_local_id(0) + get_local_size(0)*get_local_id(1));
  int i=0,j=0;
  i=get_global_id(0);
  j=get_global_id(1);
  imIndex[llid] = j*get_global_size(0)+i;

  // check to see if the thread corresponds to an actual pixel as in some
  // cases #of threads will be more than the pixels.
  if (i>=(*exp_image_dims).z || j>=(*exp_image_dims).w) {
    return;
  }

  //----------------------------------------------------------------------------
  // Calculate ray origin, and direction
  // (make sure ray direction is never axis aligned)
  //----------------------------------------------------------------------------
  float ray_ox, ray_oy, ray_oz, ray_dx, ray_dy, ray_dz;
  calc_scene_ray(linfo, camera, i, j, &ray_ox, &ray_oy, &ray_oz, &ray_dx, &ray_dy, &ray_dz);  

  //----------------------------------------------------------------------------
  // we know i,j map to a point on the image, have calculated ray
  // BEGIN RAY TRACE
  //----------------------------------------------------------------------------
  //initial expected intensity from this camera angle
  uint  eint  = as_uint(exp_image[imIndex[llid]]);
  uchar echar = convert_uchar(eint);
  float expected_int = convert_float(echar)/255.0f;   
  //input from exp_img
  uint  in_eint  = as_uint(in_exp_image[imIndex[llid]]);
  uchar in_echar = convert_uchar(in_eint);
  float exp_intensity= convert_float(in_echar)/255.0f;
  //input from actual image
  float intensity=in_image[imIndex[llid]];
  //input from probab
  uint  in_prob_eint  = as_uint(prob_exp_image[imIndex[llid]]);
  uchar in_prob_echar = convert_uchar(in_prob_eint);
  float expected_prob_int= convert_float(in_prob_echar)/255.0f;
  AuxArgs aux_args; 
  aux_args.alpha = alpha_array; aux_args.mog = mixture_array; 
  aux_args.expint = &exp_intensity;
  aux_args.intensity = &intensity; 
  aux_args.exp_prob_int = &expected_prob_int; 
  
  cast_ray( i, j,
            ray_ox, ray_oy, ray_oz,
            ray_dx, ray_dy, ray_dz,

            //scene info
            linfo, tree_array, 
            
            //utility info
            local_tree, bit_lookup, cumsum, &vis,

            //RENDER SPECIFIC ARGS
            aux_args);
  
  //expected image gets rendered
  exp_image[imIndex[llid]] =  rgbaFloatToInt((float4) 1.0f/(1.0f+expected_int)); //expected_int;
  prob_exp_image[imIndex[llid]] =  rgbaFloatToInt((float4) 1.0f/(1.0f+expected_prob_int)); //expected_int;
  vis_image[imIndex[llid]] = vis;
}
#endif

