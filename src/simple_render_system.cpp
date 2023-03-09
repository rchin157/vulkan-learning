#include "simple_render_system.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <stdexcept>
#include <array>

namespace lve
{
    struct SimplePushConstantData
    {
        glm::mat2 transform{1.f};
        glm::vec2 offset;
        alignas(16) glm::vec3 color;
    };

    SimpleRenderSystem::SimpleRenderSystem(LveDevice &device, VkRenderPass renderPass) : lveDevice{device}
    {
        createPipelineLayout();
        createPipeline(renderPass);
    }

    SimpleRenderSystem::~SimpleRenderSystem()
    {
        vkDestroyPipelineLayout(lveDevice.device(), pipelinelayout, nullptr);
    }

    void SimpleRenderSystem::createPipelineLayout()
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelinelayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create pipeline layout");
        }
    }

    void SimpleRenderSystem::createPipeline(VkRenderPass renderPass)
    {
        assert(pipelinelayout != nullptr && "Cannot create pipeline before pipeline layout");

        // auto pipelineConfig = LvePipeline::defaultPipelineConfigInfo(lveSwapChain->width(), lveSwapChain->height());
        PipelineConfigInfo pipelineConfig{};
        LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelinelayout;
        lvePipeline = std::make_unique<LvePipeline>(lveDevice, "shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv", pipelineConfig);
    }

    void SimpleRenderSystem::renderGameObjects(VkCommandBuffer commandBuffer, std::vector<LveGameObject> &gameObjects)
    {
        // update
        int i = 0;
        for (auto &obj : gameObjects)
        {
            i += 1;
            obj.transform2d.rotation = glm::mod<float>(obj.transform2d.rotation + 0.0001f * i, glm::two_pi<float>());
        }

        // render
        lvePipeline->bind(commandBuffer);
        for (auto &obj : gameObjects)
        {
            SimplePushConstantData push{};
            push.offset = obj.transform2d.translation;
            push.color = obj.color;
            push.transform = obj.transform2d.mat2();

            vkCmdPushConstants(commandBuffer, pipelinelayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
            obj.model->bind(commandBuffer);
            obj.model->draw(commandBuffer);
        }
    }

    void SimpleRenderSystem::sierpinski(std::vector<LveModel::Vertex> &vertices, int depth, glm::vec2 left, glm::vec2 right, glm::vec2 top)
    {
        if (depth <= 0)
        {
            vertices.push_back({left, {0.0f, 0.0f, 1.0f}});
            vertices.push_back({top, {1.0f, 0.0f, 0.0f}});
            vertices.push_back({right, {0.0f, 1.0f, 0.0f}});
        }
        else
        {
            auto leftTop = 0.5f * (left + top);
            auto rightTop = 0.5f * (right + top);
            auto leftRight = 0.5f * (left + right);
            sierpinski(vertices, depth - 1, leftTop, rightTop, top);
            sierpinski(vertices, depth - 1, left, leftRight, leftTop);
            sierpinski(vertices, depth - 1, leftRight, right, rightTop);
        }
    }
} // namespace lve