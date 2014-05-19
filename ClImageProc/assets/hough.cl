constant int THETA_STEP_DEG = 1;
constant int THETA_STEP_MAX = 180;
constant float DEG2RAD = 0.01745329252f;

/**
 * Performs a hough transform.
 * Each binary "1" pixel is transformed to polar coordinate space
 *     r = x * cos(theta) + y * sin(theta)
 * for each theta [0, THETA_STEP_MAX].
 */
__kernel void cl_hough(__read_only image2d_t srcImg,
                       __global uint *accSpace,     /* accumulator space (1D buffer) of size accSpaceW * accSpaceH  */
                       sampler_t sampler,
                       int accSpaceW,   int accSpaceH,
                       int imgW,        int imgH)
{
    int accSpaceWHalf = accSpaceW / 2;
    int2 absPos = (int2)(get_global_id(0), get_global_id(1));   /* absolute position */
    float2 pos = (float2)(absPos.x - (float)imgW / 2.0f,        /* rel. pos. with origin at image center */
                          absPos.y - (float)imgH / 2.0f);
    
    float bin = read_imagef(srcImg, sampler, absPos).x; /* get binary value of R channel at this pixel position */
    
    if (bin > 0.0f) {
        /* Cast votes in hough accumulator space for each possible line orientation */
        for (int thetaStep = 0; thetaStep < THETA_STEP_MAX; thetaStep += THETA_STEP_DEG) {
            float theta = (float)thetaStep * DEG2RAD;   /* convert to radians */
            
            /* calculate r (distance from origin) */
            int r = (int)(pos.x * cos(theta) + pos.y * sin(theta));
            
            /* cast vote if we are inside the accumlator space */
            if (abs(r) > 2 && r >= -accSpaceWHalf && r < accSpaceWHalf) {
                size_t accPos = thetaStep * accSpaceW + r + accSpaceWHalf;
                atomic_inc(&accSpace[accPos]); /* IMPORTANT: use atomic incrementation! */
            }
        }
    }
}