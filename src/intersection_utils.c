/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   intersection_utils.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abiri <abiri@student.1337.ma>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/26 07:22:58 by abenaiss          #+#    #+#             */
/*   Updated: 2024/06/18 15:45:30 by abiri            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "rtv1.h"

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
			rtv->row++;//rtv->pixel_size;
		}
		rtv->column++;
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
	ft_init_thread_manager(rtv);
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
