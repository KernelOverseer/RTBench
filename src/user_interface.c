/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   user_interface.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: azarzor <azarzor@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/02/14 12:36:40 by abiri             #+#    #+#             */
/*   Updated: 2020/03/04 21:48:23 by azarzor          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "rtv1.h"

int			ft_button_toggle_boolean(void *arg, int status, t_rtv *env)
{
	unsigned int	*boolean;

	if (!arg)
		return (0);
	boolean = arg;
	if (status)
		*boolean = 0;
	else
		*boolean = 1;
	ft_init_rendrering(env);
	return (*boolean);
}

t_button	*ft_new_button(char *text, t_button_handler *handler,
	void *arg, t_coor position)
{
	t_button	*result;

	if (!(result = ft_memalloc(sizeof(t_button))))
		return (NULL);
	result->handler = handler;
	result->arg = arg;
	result->position = position;
	result->text = text;
	result->status = 0;
	return (result);
}

void		ft_activate_button(t_button *button, t_rtv *env)
{
	if (button->handler)
		button->status = button->handler(button->arg, button->status, env);
}

int			ft_click_buttons(int mouse_button, int x, int y, t_rtv *env)
{
	t_list_head	*buttons;
	t_button	*button;
	int			clicked;

	clicked = 0;
	(void)mouse_button;
	buttons = &env->buttons;
	buttons->iterator = buttons->first;
	while ((button = ttslist_iter_content(buttons)))
	{
		if (x > button->position.x && x < button->position.x + BUTTON_WIDTH &&
			y > button->position.y && y < button->position.y + BUTTON_HEIGHT)
		{
			ft_activate_button(button, env);
			clicked = 1;
		}
	}
	if (!clicked)
		ft_change_lookat(mouse_button, x, y, env);
	return (0);
}