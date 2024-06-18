/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mlx_managing.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abiri <abiri@student.1337.ma>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/02/29 03:31:17 by abenaiss          #+#    #+#             */
/*   Updated: 2024/06/18 15:55:09 by abiri            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "rtv1.h"
#include "time.h"

void		ft_put_pixel(t_rtv *rtv, int color)
{
	rtv->mlx.img.data[
			(int)(rtv->column * rtv->scene.width + rtv->row)] = color;
}

void		ft_init_rendrering(t_rtv *rtv)
{
	rtv->anti_aliasing = rtv->scene.aa;
	ft_memset(&rtv->options, 1, sizeof(rtv->options));
}

static t_rect				ft_get_thread_rect(t_threads_manager thread_info)
{
	t_rect	result;

	result.start_y = (thread_info.in_thread_index / (DEFAULT_CLUSTERS_X)) * thread_info.cluster_height;
	result.start_x = (thread_info.in_thread_index % DEFAULT_CLUSTERS_X) * thread_info.cluster_width;
	result.end_x = result.start_x + thread_info.cluster_width;
	result.end_y = result.start_y + thread_info.cluster_height;
	return (result);
}

int					ft_check_empty_node(t_threads_manager *thread_info, int x, int y, t_rect *rect)
{
	if (x < 0 || x >= DEFAULT_CLUSTERS_X || y < 0 || y >= DEFAULT_CLUSTERS_Y)
	{
		printf("OUT OF BOUND\n");
		return (0);
	}
	if (thread_info->thread_grid[y][x] == 0)
	{
		rect->start_x = x * thread_info->cluster_width;
		rect->start_y = y * thread_info->cluster_height;
		rect->end_x = rect->start_x + thread_info->cluster_width;
		rect->end_y = rect->start_y + thread_info->cluster_height;
		thread_info->thread_grid[y][x] = 1;
		return (1);
	}
	return (0);	
}


static	void put_pixel(t_rtv *rtv, int x, int y, int color)
{
	rtv->mlx.img.data[(int)(y * rtv->scene.width + x)] = color;
}

void	ft_draw_rect(t_rect thread_rect, t_rtv *rtv)
{
	int i;

	i = thread_rect.start_x;
	pthread_mutex_lock(&g_render_mutex);
	while (i < thread_rect.end_x)
	{
		put_pixel(rtv, i, thread_rect.start_y, 0xFF00FF);
		put_pixel(rtv, i, thread_rect.end_y - 1, 0xFF00FF);
		i++;
	}
	i = thread_rect.start_y;
	while (i < thread_rect.end_y)
	{
		put_pixel(rtv, thread_rect.start_x, i, 0xFF00FF);
		put_pixel(rtv, thread_rect.end_x - 1, i, 0xFF00FF);
		i++;
	}
	pthread_mutex_unlock(&g_render_mutex);
}

pthread_mutex_t	g_next;

void		*ft_multi_threaded_ray_loop(void *data)
{
	t_rtv	*rtv;
	t_rtv	*main_rtv;
	t_color	rgb;
	t_rect	thread_rect;

	rtv = data;
	main_rtv = rtv->main_rtv;
	while (rtv->thread_manager.in_thread_index <
		rtv->thread_manager.num_clusters)
	{
		thread_rect = ft_get_thread_rect(rtv->thread_manager);
		rtv->column = thread_rect.start_y;
		while (rtv->column < thread_rect.end_y)
		{
			rtv->row = thread_rect.start_x;
			while (rtv->row < thread_rect.end_x)
			{
				rgb = (t_color){0, 0, 0};
				ft_color_best_node(rtv, rgb);
				rtv->row++;
			}
			rtv->column++;
		}
		pthread_mutex_lock(&g_next);
		rtv->thread_manager.in_thread_index = main_rtv->thread_manager.next_cluster_index++;
		pthread_mutex_unlock(&g_next);
	}
	pthread_mutex_lock(&g_next);
	main_rtv->thread_manager.idle_threads++;
	pthread_mutex_unlock(&g_next);
	return (NULL);
}

void		multi_threaded_ray_shooter(t_rtv *rtv)
{
	pthread_t	*thread;
	t_rtv		*rtv_cpy;
	int			i;


	thread = malloc(sizeof(pthread_t) * rtv->thread_manager.num_threads);
	rtv_cpy = malloc(sizeof(t_rtv) * rtv->thread_manager.num_threads);
	i = -1;
	printf("RENDERING WITH %d THREADS\n", rtv->thread_manager.num_threads);
	while (++i < rtv->thread_manager.num_threads)
	{
		rtv_cpy[i] = *rtv;
		rtv_cpy[i].main_rtv = rtv;
		rtv_cpy[i].min_w = i;
		rtv_cpy[i].max_w = rtv->scene.width;
		rtv_cpy[i].thread_index = i;
		rtv_cpy[i].thread_manager.in_thread_index = rtv->thread_manager.next_cluster_index++;
		pthread_create(&thread[i], NULL, ft_multi_threaded_ray_loop, &rtv_cpy[i]);
	}
	printf("CREATED ALL THREADS\n");
	while (i--)
		pthread_join(thread[i], NULL);
}

void		ft_render_thread(t_rtv *rtv)
{
	gettimeofday(&rtv->thread_manager.start_time, NULL);
	multi_threaded_ray_shooter(rtv);
}

void		ft_init_thread_manager(t_rtv *rtv)
{
	rtv->thread_manager.cluster_height = rtv->scene.height / DEFAULT_CLUSTERS_Y;
	rtv->thread_manager.cluster_width = rtv->scene.width / DEFAULT_CLUSTERS_X;
	rtv->thread_manager.num_clusters = DEFAULT_CLUSTERS_X * DEFAULT_CLUSTERS_Y;
	rtv->thread_manager.num_threads = NUM_THREAD;//sysconf(_SC_NPROCESSORS_ONLN);
	rtv->thread_manager.next_cluster_index = 0;
	ft_render_thread(rtv);
}