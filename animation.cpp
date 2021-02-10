#include "animation.hpp"

std::map<const char*, Animation::Anim*> Animation::trackedAnimations;

TIME_POINT Animation::now()
{
    auto x = std::chrono::high_resolution_clock::now();
    return x;
};

double Animation::delta(TIME_POINT after, TIME_POINT before) // after > before, in seconds
{
    return std::chrono::duration_cast<std::chrono::microseconds>(after - before).count() / (double)1e6;
};

Animation::Anim* Animation::get(const char* name)
{
    return trackedAnimations.at(name);
}

double Animation::age(Animation::Anim* anim)
{
    return delta(now(), anim->changed);
}

double Animation::age(const char* name)
{
    Anim* a = get(name);
    if (!a) return 1.0;
    return age(a);
}

Animation::Anim* Animation::newAnimation(const char* name, int state)
{
    Anim* a = new Anim(state, now() - std::chrono::hours(24)); // it changed a long time ago
    trackedAnimations.insert(std::make_pair(name, a));
    return a;
}

void Animation::deleteAnimation(const char* name)
{
    auto iterator = trackedAnimations.find(name);
    if (iterator == trackedAnimations.end()) return;
    Anim* a = iterator->second;
    trackedAnimations.erase(iterator);
    delete a;
}

bool Animation::changed(const char* name, int newState)
{
    return changed(get(name), newState);
}

bool Animation::changed(Animation::Anim* anim, int newState)
{
    if (!anim) return false;
    if (anim->state == newState) return false;
    anim->state = newState;
    anim->changed = now();
    return true;
}

float Animation::animate(double timePassed, double animationTime, Animation::Interpolation i)
{
    float f = (float)(timePassed / animationTime);
    if (f >= 1.f) return 1.f;
    if (f <= 0.f) return 0.f;

    switch (i)
    {
    case Interpolation::linear:
        return f;
    case Interpolation::easeInOutQuint:
        return f < 0.5f ? 16.f * f * f * f * f * f : 1.f - powf(-2.f * f + 2.f, 5.f) / 2.f;
    case Interpolation::easeInOutQuart:
    default:
        return f < 0.5f ? 2.f * f * f : 1.f - powf(-2.f * f + 2.f, 2.f) / 2.f;
    }
}

float Animation::lerp(float a, float b, float f)
{
    return (b - a) * f + a;
}
