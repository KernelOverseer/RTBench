/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mlx_managing.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abiri <abiri@student.1337.ma>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/02/29 03:31:17 by abenaiss          #+#    #+#             */
/*   Updated: 2021/06/29 16:17:23 by abiri            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "rtv1.h"
#include "time.h"

void		ft_put_pixel(t_rtv *rtv, int color)
{
	// int i;
	// int j;

	// i = -1;
	// while (++i < rtv->render_offset)
	// {
	// 	j = -1;
	// 	while (++j < rtv->render_y_offset)
	// 	{
	// 		if (rtv->row >= 0 && rtv->row < rtv->scene.width &&
	// 			rtv->column >= 0 && rtv->column < rtv->scene.height)
	// 			rtv->mlx.img.data[(int)
	// 			(((int)(rtv->column - j) % rtv->scene.height)
	// 			* rtv->scene.width + (int)(rtv->row - i)
	// 			% rtv->scene.width)] = color;
	// 	}
	// }
	// if (rtv->row >= 0 && rtv->row < rtv->scene.width &&
	// 	rtv->column >= 0 && rtv->column < rtv->scene.height)
	rtv->mlx.img.data[
			(int)(rtv->column * rtv->scene.width + rtv->row)] = color;
}

void		*ft_draw_thread(void *data)
{
	t_rtv	*rtv;

	rtv = data;
	while (1)
	{
		mlx_put_image_to_window(rtv->mlx.mlx_ptr, rtv->mlx.win,
				rtv->mlx.img.img_ptr, 0, 0);
	}
	return (NULL);
}

static long long	calculate_score(t_rtv *rtv)
{
	long long duration = ((rtv->thread_manager.end_time.tv_sec -
		rtv->thread_manager.start_time.tv_sec) * 1000000 +
		rtv->thread_manager.end_time.tv_usec - rtv->thread_manager.start_time.tv_usec);
	
	long long score = 1000000000000L / duration;
	return (score);
}

static int	ft_frame_loop(void *arg)
{
	t_rtv	*rtv;

	rtv = arg;
	// (void)arg;
	// printf("DRAWING\n");
	pthread_mutex_lock(&g_render_mutex);
	if (rtv->thread_manager.idle_threads == rtv->thread_manager.num_threads)
	{
		rtv->thread_manager.idle_threads = 0;
		gettimeofday(&rtv->thread_manager.end_time, NULL);
		printf("your score is : %lld\n", calculate_score(rtv));
		exit(0);
	}
	mlx_put_image_to_window(rtv->mlx.mlx_ptr, rtv->mlx.win,
			rtv->mlx.img.img_ptr, 0, 0);
	pthread_mutex_unlock(&g_render_mutex);
	// ft_draw_buttons(rtv);
	return (0);
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

// static int					ft_get_empty_node(t_threads_manager *thread_info, int x, int y, t_rect *rect)
// {
// 	if (x < 0 || x >= DEFAULT_CLUSTERS_X || y < 0 || y >= DEFAULT_CLUSTERS_Y)
// 	{
// 		printf("OUT OF BOUND\n");
// 		return (0);
// 	}
// 	for (int i = 0; i < DEFAULT_CLUSTERS_Y; i++)
// 	{
// 		for (int j = 0; i < DEFAULT_CLUSTERS_Y; j++)
// 		{
			
// 		}
// 	}
// 	return (0);
// }

// static	t_rect				ft_get_next_empty_rect(t_threads_manager *thread_info)
// {
// 	t_rect	result;
// 	int		sx;
// 	int		sy;

// 	sx = DEFAULT_CLUSTERS_X / 2;
// 	sy = DEFAULT_CLUSTERS_Y / 2;

// 	if (ft_get_empty_node(thread_info, sx, sy, &result) == 0)
// 		printf("POSITION NOT FOUND\n");
// 	return (result);
// }

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
		// pthread_mutex_lock(&g_next);
		// thread_rect = ft_get_next_empty_rect(&(main_rtv->thread_manager));
		// pthread_mutex_unlock(&g_next);
		ft_draw_rect(thread_rect, rtv);
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
	// while (i--)
		// pthread_join(thread[i], NULL);
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
	rtv->thread_manager.num_threads = sysconf(_SC_NPROCESSORS_ONLN);
	rtv->thread_manager.next_cluster_index = 0;
	ft_render_thread(rtv);
}

void		ft_init_win(t_rtv *rtv)
{
	// pthread_t	draw_thread;

	pthread_mutex_init(&g_next, NULL);
	ft_init_rendrering(rtv);
	rtv->mlx.mlx_ptr = mlx_init();
	rtv->mlx.img.img_ptr = mlx_new_image(rtv->mlx.mlx_ptr,
			rtv->scene.width, rtv->scene.height);
	rtv->mlx.img.data = (int*)mlx_get_data_addr(rtv->mlx.img.img_ptr,
			&rtv->mlx.img.bpp, &rtv->mlx.img.size_l, &rtv->mlx.img.endian);
	rtv->mlx.win = mlx_new_window(rtv->mlx.mlx_ptr, rtv->scene.width,
		rtv->scene.height, "RT-Bench");
	rtv->mlx.img.height = rtv->scene.height;
	rtv->mlx.img.width = rtv->scene.width;


	ft_init_thread_manager(rtv);
	// pthread_create(&draw_thread, NULL, ft_draw_thread, rtv);

	mlx_hook(rtv->mlx.win, 2, 0, &ft_key_stroke, rtv);
	mlx_hook(rtv->mlx.win, 17, 1, (*ft_exit), rtv);
	mlx_mouse_hook(rtv->mlx.win, ft_click_buttons, rtv);
	mlx_loop_hook(rtv->mlx.mlx_ptr, &ft_frame_loop, rtv);
	mlx_loop(rtv->mlx.mlx_ptr);

	// pthread_join(draw_thread, NULL);
}
