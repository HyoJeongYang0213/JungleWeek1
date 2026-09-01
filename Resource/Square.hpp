#pragma once 
#include "VertexSimple.hpp"

// 큐브를 하드 코딩
VertexSimple cube_vertices[] =
{
	// Front face (Z+)
	{ -1.0f, -1.0f,  1.0f,  1.0f, 0.0f, 0.0f, 1.0f }, // Bottom-left (red)
	{ -1.0f,  1.0f,  1.0f,  1.0f, 1.0f, 0.0f, 1.0f }, // Top-left (yellow)
	{  1.0f, -1.0f,  1.0f,  0.0f, 1.0f, 0.0f, 1.0f }, // Bottom-right (green)
	{ -1.0f,  1.0f,  1.0f,  1.0f, 1.0f, 0.0f, 1.0f }, // Top-left (yellow)
	{  1.0f,  1.0f,  1.0f,  0.0f, 0.0f, 1.0f, 1.0f }, // Top-right (blue)
	{  1.0f, -1.0f,  1.0f,  0.0f, 1.0f, 0.0f, 1.0f }, // Bottom-right (green)

	// Back face (Z-)
	{ -1.0f, -1.0f, -1.0f,  0.0f, 1.0f, 1.0f, 1.0f }, // Bottom-left (cyan)
	{  1.0f, -1.0f, -1.0f,  1.0f, 0.0f, 1.0f, 1.0f }, // Bottom-right (magenta)
	{ -1.0f,  1.0f, -1.0f,  0.0f, 0.0f, 1.0f, 1.0f }, // Top-left (blue)
	{ -1.0f,  1.0f, -1.0f,  0.0f, 0.0f, 1.0f, 1.0f }, // Top-left (blue)
	{  1.0f, -1.0f, -1.0f,  1.0f, 0.0f, 1.0f, 1.0f }, // Bottom-right (magenta)
	{  1.0f,  1.0f, -1.0f,  1.0f, 1.0f, 0.0f, 1.0f }, // Top-right (yellow)

	// Left face (X-)
	{ -1.0f, -1.0f, -1.0f,  1.0f, 0.0f, 1.0f, 1.0f }, // Bottom-left (purple)
	{ -1.0f,  1.0f, -1.0f,  0.0f, 0.0f, 1.0f, 1.0f }, // Top-left (blue)
	{ -1.0f, -1.0f,  1.0f,  0.0f, 1.0f, 0.0f, 1.0f }, // Bottom-right (green)
	{ -1.0f,  1.0f, -1.0f,  0.0f, 0.0f, 1.0f, 1.0f }, // Top-left (blue)
	{ -1.0f,  1.0f,  1.0f,  1.0f, 1.0f, 0.0f, 1.0f }, // Top-right (yellow)
	{ -1.0f, -1.0f,  1.0f,  0.0f, 1.0f, 0.0f, 1.0f }, // Bottom-right (green)

	// Right face (X+)
	{  1.0f, -1.0f, -1.0f,  1.0f, 0.5f, 0.0f, 1.0f }, // Bottom-left (orange)
	{  1.0f, -1.0f,  1.0f,  0.5f, 0.5f, 0.5f, 1.0f }, // Bottom-right (gray)
	{  1.0f,  1.0f, -1.0f,  0.5f, 0.0f, 0.5f, 1.0f }, // Top-left (purple)
	{  1.0f,  1.0f, -1.0f,  0.5f, 0.0f, 0.5f, 1.0f }, // Top-left (purple)
	{  1.0f, -1.0f,  1.0f,  0.5f, 0.5f, 0.5f, 1.0f }, // Bottom-right (gray)
	{  1.0f,  1.0f,  1.0f,  0.0f, 0.0f, 0.5f, 1.0f }, // Top-right (dark blue)

	// Top face (Y+)
	{ -1.0f,  1.0f, -1.0f,  0.0f, 1.0f, 0.5f, 1.0f }, // Bottom-left (light green)
	{ -1.0f,  1.0f,  1.0f,  0.0f, 0.5f, 1.0f, 1.0f }, // Top-left (cyan)
	{  1.0f,  1.0f, -1.0f,  0.5f, 1.0f, 1.0f, 1.0f }, // Bottom-right (white)
	{ -1.0f,  1.0f,  1.0f,  0.0f, 0.5f, 1.0f, 1.0f }, // Top-left (cyan)
	{  1.0f,  1.0f,  1.0f,  0.5f, 0.5f, 0.0f, 1.0f }, // Top-right (brown)
	{  1.0f,  1.0f, -1.0f,  0.5f, 1.0f, 1.0f, 1.0f }, // Bottom-right (white)

	// Bottom face (Y-)
	{ -1.0f, -1.0f, -1.0f,  0.5f, 0.5f, 0.0f, 1.0f }, // Bottom-left (brown)
	{ -1.0f, -1.0f,  1.0f,  1.0f, 0.0f, 0.0f, 1.0f }, // Top-left (red)
	{  1.0f, -1.0f, -1.0f,  1.0f, 0.0f, 0.5f, 1.0f }, // Bottom-right (purple)
	{ -1.0f, -1.0f,  1.0f,  1.0f, 0.0f, 0.0f, 1.0f }, // Top-left (red)
	{  1.0f, -1.0f,  1.0f,  0.0f, 1.0f, 0.0f, 1.0f }, // Top-right (green)
	{  1.0f, -1.0f, -1.0f,  1.0f, 0.0f, 0.5f, 1.0f }, // Bottom-right (purple)
};