/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   intersection_utils.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abiri <abiri@student.1337.ma>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/26 07:22:58 by abenaiss          #+#    #+#             */
/*   Updated: 2021/06/28 17:53:28 by abiri            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "rtv1.h"

// static void			ft_display_loading(t_rtv *rtv)
// {
// 	int x;
// 	int y;
// 	int	max;

// 	y = rtv->scene.height - 5;
// 	x = 0;
// 	max = (double)rtv->scene.width - ((double)((rtv->render_y_offset)
// 	* rtv->pixel_size + (rtv->render_offset)) / (double)(rtv->pixel_size
// 	* rtv->pixel_size + rtv->pixel_size)) * (double)rtv->scene.width;
// 	max = (max == rtv->scene.width) ? 0 : max;
// 	while (x < max)
// 	{
// 		y = rtv->scene.height - 5;
// 		while (y < rtv->scene.height)
// 		{
// 			if (x >= 0 && x < rtv->scene.width &&
// 					y >= 0 && y < rtv->scene.height)
// 				rtv->mlx.img.data[(int)(y * rtv->scene.width + x)] = 0xFF00FF;
// 			y++;
// 		}
// 		x++;
// 	}
// }

static t_rect				ft_get_thread_rect(t_threads_manager thread_info)
{
	t_rect	result;

	result.start_y = (thread_info.in_thread_index / (DEFAULT_CLUSTERS_X + 1)) * thread_info.cluster_height;
	result.start_x = (thread_info.in_thread_index % DEFAULT_CLUSTERS_X) * thread_info.cluster_width;
	result.end_x = result.start_x + thread_info.cluster_width;
	result.end_y = result.start_y + thread_info.cluster_height;
	return (result);
}


static void			*ft_ray_loop(void *data)
{
	t_color	rgb;
	t_rtv	*rtv;
	t_rect	thread_rect;

	rtv = data;
	thread_rect = ft_get_thread_rect(rtv->thread_manager);
	printf("RES : %d | %d\n", rtv->scene.width, rtv->scene.height);
	printf("%d %d | %d %d\n", thread_rect.start_x, thread_rect.end_x, thread_rect.start_y, thread_rect.end_y);
	rtv->column = thread_rect.start_y;
	while (rtv->column < thread_rect.end_y)
	{
		rtv->row = thread_rect.start_x;
		while (rtv->row < thread_rect.end_x)
		{
			rgb = (t_color){0, 0, 0};
			if (rtv->scene.dof && rtv->options.depth_of_field)
				ft_color_best_node_dof(rtv, rgb);
			ft_color_best_node(rtv, rgb);
			mlx_put_image_to_window(rtv->mlx.mlx_ptr, rtv->mlx.win,
				rtv->mlx.img.img_ptr, 0, 0);
			rtv->row++;//rtv->pixel_size;
		}
		rtv->column++;
		//printf("\ec");
		//printf("PROGRESS : %2.2f%%\n", ((double)g_rendered_pixels / (double)rtv->total_pixels) * 100.0);
		// ft_display_loading(rtv);
	}
	printf("EXITED FROM THREAD %d\n", rtv->thread_index);
	return (NULL);
}

int					ft_ray_shooter(t_rtv *rtv)
{
	pthread_t	thread[NUM_THREAD];
	t_rtv		rtv_cpy[NUM_THREAD];
	int			i;

	i = -1;
	printf("RENDERING WITH %d THREADS\n", NUM_THREAD);
	while (++i < NUM_THREAD)
	{
		rtv_cpy[i] = *rtv;
		rtv_cpy[i].min_w = i;
		rtv_cpy[i].max_w = rtv->scene.width;
		rtv_cpy[i].thread_index = i;
		rtv_cpy[i].thread_manager.in_thread_index = rtv->thread_manager.next_cluster_index++;
		pthread_create(&thread[i], NULL, ft_ray_loop, &rtv_cpy[i]);
	}
	while (i--)
		pthread_join(thread[i], NULL);
	// ft_update_offset(rtv);
	return (0);
}
